/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#ifndef ELYSIAN_MAP_FORMAT_HPP
#define ELYSIAN_MAP_FORMAT_HPP
#include <array>
#include <cstdint>
#include <cstring>
#include <cstddef>

namespace elysian::mapformat {
constexpr unsigned LegacyLayers=3, MaxLayers=4;
// Version 1 prefixes the legacy header with an unmistakable signature, then
// uint16 version and layer count. All integers, including entries, are LE.
constexpr std::array<unsigned char,8> Magic={'E','S','M','A','P','\r','\n',0x1a};
struct Header {
    std::array<char,50> directory{}, name{};
    unsigned width=0,height=0,layers=LegacyLayers;
    std::size_t bytes=104;
    std::size_t entries() const { return std::size_t(width)*height*layers; }
    bool valid() const {
        // Matches the renderer's supported map dimensions and index capacity.
        return width>0 && height>0 && width<=200 && height<=200 &&
            (layers==LegacyLayers || layers==MaxLayers);
    }
};
inline unsigned u16(const unsigned char* p) { return unsigned(p[0])|(unsigned(p[1])<<8); }
inline void put16(unsigned char* p,unsigned v) { p[0]=v&255; p[1]=(v>>8)&255; }
template<class Read> bool readHeader(Read read,Header& h) {
    std::array<unsigned char,104> b{};
    if(!read(b.data(),8)) return false;
    if(std::memcmp(b.data(),Magic.data(),8)==0) {
        unsigned char version[4];
        if(!read(version,4) || u16(version)!=1) return false;
        h.layers=u16(version+2); h.bytes=116;
        if(!read(b.data(),b.size())) return false;
    } else {
        h.layers=LegacyLayers; h.bytes=104;
        if(!read(b.data()+8,b.size()-8)) return false;
    }
    std::memcpy(h.directory.data(),b.data(),50);
    std::memcpy(h.name.data(),b.data()+50,50);
    h.width=u16(b.data()+100); h.height=u16(b.data()+102);
    return h.valid();
}
template<class Write> bool writeHeader(Write write,const Header& h) {
    if(!h.valid()) return false;
    if(h.layers!=LegacyLayers) {
        unsigned char version[4]; put16(version,1); put16(version+2,h.layers);
        if(!write(Magic.data(),Magic.size()) || !write(version,4)) return false;
    }
    std::array<unsigned char,104> b{};
    std::memcpy(b.data(),h.directory.data(),50); std::memcpy(b.data()+50,h.name.data(),50);
    put16(b.data()+100,h.width); put16(b.data()+102,h.height);
    return write(b.data(),b.size());
}
// Stream in small blocks: no second full-map buffer, also on Dreamcast.
template<class Read,class Set> bool readEntries(Read read,std::size_t count,Set set) {
    std::array<unsigned char,1024> b{};
    for(std::size_t start=0;start<count;) {
        const auto n=(count-start<256)?count-start:256;
        if(!read(b.data(),n*4)) return false;
        for(std::size_t i=0;i<n;++i) {
            const auto* p=b.data()+i*4;
            set(start+i,uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24));
        }
        start+=n;
    }
    return true;
}
template<class Write,class Get> bool writeEntries(Write write,std::size_t count,Get get) {
    std::array<unsigned char,1024> b{};
    for(std::size_t start=0;start<count;) {
        const auto n=(count-start<256)?count-start:256;
        for(std::size_t i=0;i<n;++i) {
            const uint32_t v=get(start+i);
            for(unsigned j=0;j<4;++j) b[i*4+j]=(v>>(j*8))&255;
        }
        if(!write(b.data(),n*4)) return false;
        start+=n;
    }
    return true;
}
}
#endif
