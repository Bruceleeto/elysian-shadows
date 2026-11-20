/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#include <AssetIO/level/elysian_map_format.hpp>
#include <AssetIO/level/elysian_terrain_geometry.hpp>
#include <AssetIO/level/elysian_tile_collision.hpp>
#include <ESToolkit/core/tk_3d_tile_brush.h>
#include <ESToolkit/core/tk_3d_paint_stroke.h>
#include <QUndoStack>
#include <QTemporaryDir>
#include <libGyro/gyro_file_api.h>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cstdlib>
using elysian::TileEntry;
namespace mf=elysian::mapformat;
static void check(bool ok,const char* message) {
    if(!ok) { std::fprintf(stderr,"FAIL: %s\n",message); std::exit(1); }
}
struct Map {
    mf::Header header;
    std::vector<TileEntry> tiles;
    Map(unsigned layers=4) {
        header.width=8; header.height=12; header.layers=layers; tiles.resize(header.entries());
    }
    TileEntry& at(int l,int x,int y) { return tiles.at((l*header.height+y)*header.width+x); }
    std::vector<unsigned char> save() const {
        std::vector<unsigned char> bytes;
        const auto write=[&](const void* p,std::size_t n) {
            const auto* b=static_cast<const unsigned char*>(p); bytes.insert(bytes.end(),b,b+n); return true;
        };
        check(mf::writeHeader(write,header) && mf::writeEntries(write,tiles.size(),[&](std::size_t i){return tiles[i].getValue();}),"save map");
        return bytes;
    }
    bool load(const std::vector<unsigned char>& bytes) {
        std::size_t offset=0;
        const auto read=[&](void* p,std::size_t n) {
            if(n>bytes.size()-offset) return false;
            std::memcpy(p,bytes.data()+offset,n); offset+=n; return true;
        };
        mf::Header h;
        if(!mf::readHeader(read,h) || bytes.size()!=h.bytes+h.entries()*4) return false;
        std::vector<TileEntry> values(h.entries());
        if(!mf::readEntries(read,h.entries(),[&](std::size_t i,uint32_t v){values[i]=TileEntry(v);})) return false;
        header=h; tiles=std::move(values); return true;
    }
};
class Command {
    Map& map; int layer;
    struct Change { QPoint point; TileEntry before,after; };
    std::vector<Change> changes;
public:
    Command(Map& m,int l):map(m),layer(l) {}
    void setCombine(bool) {}
    bool addTile(QPoint p,TileEntry after) { changes.push_back({p,map.at(layer,p.x(),p.y()),after}); return true; }
    void undo() { for(auto i=changes.rbegin();i!=changes.rend();++i) map.at(layer,i->point.x(),i->point.y())=i->before; }
    void redo() { for(const auto& c:changes) map.at(layer,c.point.x(),c.point.y())=c.after; }
};
static void formatChecks() {
    for(unsigned layers:{3u,4u}) {
        Map source(layers),restored;
        for(std::size_t i=0;i<source.tiles.size();++i) source.tiles[i]=TileEntry(uint32_t(i*12345));
        const auto bytes=source.save();
        check(bytes.size()==(layers==3?104:116)+source.tiles.size()*4,"exact legacy/versioned byte size");
        check(restored.load(bytes) && restored.header.layers==layers && restored.tiles==source.tiles,"all tile bits round trip");
        check(restored.save()==bytes,"save/load/save is byte identical");
        for(std::size_t length:{std::size_t(0),std::size_t(7),std::size_t(103),bytes.size()-1}) {
            auto shortFile=bytes; shortFile.resize(length);
            check(!restored.load(shortFile) && restored.tiles==source.tiles,"truncation rejected without changing map");
        }
        auto extra=bytes; extra.push_back(0); check(!restored.load(extra),"trailing entries rejected");
        auto invalid=bytes;
        const unsigned dimensions=layers==3?100:112;
        invalid[dimensions]=0; invalid[dimensions+1]=0; check(!restored.load(invalid),"zero width rejected");
        if(layers==4) {
            invalid=bytes; invalid[8]=2; check(!restored.load(invalid),"unknown version rejected");
            invalid=bytes; invalid[10]=5; check(!restored.load(invalid),"unsupported layer count rejected");
            invalid=bytes; invalid[10]=0; check(!restored.load(invalid),"zero layers rejected");
        }
    }
    std::ifstream input("tests/fixtures/stepped_terrain/area/terrain/map.bin",std::ios::binary);
    check(bool(input),"legacy fixture exists");
    const std::vector<unsigned char> bytes{std::istreambuf_iterator<char>(input),{}};
    Map old;
    check(old.load(bytes) && old.header.layers==3 && old.save()==bytes,"existing unmodified legacy map round trips");
    const auto original=old.tiles;
    old.header.layers=4; old.tiles.resize(old.header.entries());
    Map upgraded;
    check(upgraded.load(old.save()) && std::equal(original.begin(),original.end(),upgraded.tiles.begin()),"upgrade preserves every legacy entry");
    for(std::size_t i=original.size();i<upgraded.tiles.size();++i) check(upgraded.tiles[i].getValue()==0,"new layer starts empty");
}
static void fileBackendChecks() {
    check(gyFileInit(nullptr),"initialize real libGyro file backend");
    QTemporaryDir directory;
    check(directory.isValid(),"temporary map directory");
    for(unsigned count:{3u,4u}) {
        Map map(count);
        map.tiles.back()=TileEntry(0x213456);
        const auto bytes=map.save();
        const auto path=directory.filePath(QString::number(count)+".bin").toStdString();
        { std::ofstream file(path,std::ios::binary); file.write(reinterpret_cast<const char*>(bytes.data()),bytes.size()); check(bool(file),"write temporary map"); }
        GYFile* file=nullptr;
        check(gyFileOpen(path.c_str(),"rb",&file),"libGyro opens map");
        const auto read=[&](void* data,std::size_t size) {
            std::size_t actual=0;
            return gyFileRead(file,data,size,&actual) && actual==size;
        };
        mf::Header header; long length=0;
        check(mf::readHeader(read,header) && header.layers==count,"real backend decodes layer header");
        check(gyFileSeek(file,0,SEEK_END) && gyFileTell(file,&length) &&
              std::size_t(length)==header.bytes+header.entries()*4 &&
              gyFileSeek(file,static_cast<off_t>(header.bytes),SEEK_SET),"real backend validates length and seeks to entries");
        std::vector<TileEntry> entries(header.entries());
        check(mf::readEntries(read,entries.size(),[&](std::size_t i,uint32_t value){entries[i]=TileEntry(value);}) &&
              entries==map.tiles,"real backend reads all layers including last word");
        check(gyFileClose(&file),"close map");
    }
    check(gyFileQuit(),"close file backend");
}
static void fourthLayerChecks() {
    Map map;
    for(int l=0;l<3;++l) { auto& t=map.at(l,2,3); t.setIndex(1); t.setElev(10+l); }
    map.at(0,2,3).setBlocked(true);
    const auto before=map.tiles;
    TileBrush3D brush; brush.width=brush.height=1; brush.cells.push_back({0,0,TileEntry(1)});
    const auto read=[&](int l,int x,int y){return map.at(l,x,y);};
    auto plan=planTileBrush3D(brush,{2,3,0,3,0},8,12,1,read,4);
    check(plan.valid() && plan.tiles.front().face.layer==3,"full legacy square allocates fourth layer");
    check(!planTileBrush3D(brush,{2,3,0,3,0},8,12,1,read,3).valid(),"three-layer map cannot allocate nonexistent storage");
    using Stroke=TilePaint3DStroke<Command>;
    auto stroke=std::make_unique<Stroke>(false,
        [&](int l){return std::make_unique<Command>(map,l);},
        [&](int l,QPoint p){return read(l,p.x(),p.y());},
        [&](int l,QPoint p,TileEntry t){map.at(l,p.x(),p.y())=t;});
    check(stroke->paint(plan.tiles[0].face,TileEntry(1)),"fourth-layer stroke writes");
    QUndoStack history; history.push(stroke.release());
    const auto painted=map.tiles;
    check(std::equal(before.begin(),before.begin()+3*8*12,painted.begin()),"all earlier surfaces and blocked bits preserved");
    history.undo(); check(map.tiles==before,"undo removes only new surface");
    history.redo(); check(map.tiles==painted,"redo restores fourth surface");
    Map restored; check(restored.load(map.save()) && restored.tiles==painted,"fourth-layer painting survives persistence");
    const auto floor=[&](unsigned l,int x,int y) { return map.at(l,x,y); };
    const auto support=elysian::terrain::support(8,12,4,floor,64,192,96);
    check(support && support->top==96 && support->layer==3,"fourth-layer floor has its rendered height");
    check(!elysian::terrain::support(8,12,3,floor,64,192,96),"floor test genuinely depends on fourth layer");
    auto nonsolid=map.at(3,2,3);nonsolid.setSolid(false);
    auto change=std::make_unique<Stroke>(false,
        [&](int l){return std::make_unique<Command>(map,l);},
        [&](int l,QPoint p){return read(l,p.x(),p.y());},
        [&](int l,QPoint p,TileEntry t){map.at(l,p.x(),p.y())=t;});
    check(change->paint(plan.tiles[0].face,nonsolid),"solid setting changes through the normal paint command");
    history.push(change.release());check(!map.at(3,2,3).getSolid(),"collision disabled on the painted instance");
    check(restored.load(map.save()) && !restored.at(3,2,3).getSolid(),"collision off survives save/load");
    history.undo();check(map.at(3,2,3).getSolid(),"undo restores solidity");
    history.redo();check(!map.at(3,2,3).getSolid(),"redo restores collision off");
    const auto full=planTileBrush3D(brush,{2,3,0,4,1},8,12,1,read,4);
    check(!full.valid() && full.failure==TileBrushPlan3D::Failure::Full,"fifth surface safely rejected");
}
int main() {
    formatChecks(); fileBackendChecks(); fourthLayerChecks();
    std::puts("PASS: legacy/versioned maps, corruption rejection, fourth-layer paint/undo/save/load/floor/wall and capacity");
}
