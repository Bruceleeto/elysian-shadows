/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_DIALOG_BASE_HPP
#define ELYSIAN_DIALOG_BASE_HPP

#include <vector>
#include <AssetIO/utility/elysian_asset_typedefs.hpp>
#include <AssetIO/utility/elysian_lua_object.hpp>
#include <AssetIO/crtp/elysian_dialog_screen.hpp>

#define ELYSIAN_DIALOG_PROMPT_OPTION_TEXT_BUFFER_SIZE	100
#define ELYSIAN_DIALOG_PROMPT_MAX_OPTION_COUNT			6

ELYSIAN_ASSET_TYPEDEF(Entity)
ELYSIAN_ASSET_TYPEDEF(DialogText)
ELYSIAN_ASSET_TYPEDEF(DialogOption)
ELYSIAN_ASSET_TYPEDEF(DialogPrompt)
ELYSIAN_ASSET_TYPEDEF(Dialog)

namespace elysian {

    class DialogTextBase: public LuaObject<DialogTextBase, DialogText, DialogScreen>, public DialogScreen {
    public: using LuaObjectType = LuaObject<DialogTextBase, DialogText, DialogScreen>;

        public:
    public:
       enum INPUT_TYPE { INPUT_MODAL, INPUT_ADVANCE, INPUT_NONE };
    private:
        //bool          _persist = false;
        //float         _waitTime;
        //INPUT_TYPE	_inputType = INPUT_ADVANCE;
        int             _nextIndex = -1;
    public:

        using           LuaObject<DialogTextBase, DialogText, DialogScreen>::fromTable;
        using           LuaObject<DialogTextBase, DialogText, DialogScreen>::lua_downCast;

                        DialogTextBase(void);
                        DialogTextBase(const char*const text);
                        DialogTextBase(const DialogTextBase& rhs);
        DialogTextBase& operator=(const DialogTextBase& rhs);

        void            setNextIndex(const int index);
        void            setNextIndexLua(const int index);
        int             getNextIndex(void) const;
        int             getNextIndexLua(void) const;
    };



    class DialogOptionBase: public LuaObject<DialogOptionBase, DialogOption> {
    public: using LuaObjectType = LuaObject<DialogOptionBase, DialogOption>;

        public:
    private:
        char                _text[ELYSIAN_DIALOG_PROMPT_OPTION_TEXT_BUFFER_SIZE] = {0};
        int                 _nextIndex = -1;
    public:
                            DialogOptionBase(void) = default;
                            DialogOptionBase(const char* string, const int nextIndex);
                            DialogOptionBase(const DialogOptionBase& rhs);
        DialogOptionBase&   operator=(const DialogOptionBase& rhs);

        const char*         getText(void) const;
        void                setText(const char* string);
        int                 getNextIndex(void) const;
        int                 getNextIndexLua(void) const;
        void                setNextIndex(const int index);
        void                setNextIndexLua(const int index);
    };


    class DialogPromptBase: public LuaObject<DialogPromptBase, DialogPrompt, DialogScreen>, public DialogScreen {
    public: using LuaObjectType = LuaObject<DialogPromptBase, DialogPrompt, DialogScreen>;

        public:
    private:
        std::vector<DialogOption*>  _options;

    public:

        using                       LuaObject<DialogPromptBase, DialogPrompt, DialogScreen>::fromTable;
        using                       LuaObject<DialogPromptBase, DialogPrompt, DialogScreen>::lua_downCast;

                                    DialogPromptBase(void);
                                    DialogPromptBase(const char*const text);
                                    DialogPromptBase(const DialogPromptBase& rhs);
        virtual                     ~DialogPromptBase(void);
        DialogPromptBase&           operator=(const DialogPromptBase& rhs);

        void                        addOption(const char*const text, const int nextIndex);
        unsigned                    getOptionCount(void) const;

        DialogOption*               getOption(const unsigned index) const;
        void                        setOption(const unsigned index, DialogOption* option);

    };

    class DialogBase: public LuaObject<DialogBase, Dialog> {
    public: using LuaObjectType = LuaObject<DialogBase, Dialog>;

        public:
    private:
        std::vector<DialogScreen*>	_screens;
        Entity*						_triggerEntity  = nullptr;
        int                         _startScreen    = 0;

    public:
                                    DialogBase(void) = default;
                                    DialogBase(const DialogBase& rhs);
                                    ~DialogBase(void);
        DialogBase&                 operator=(const DialogBase& rhs);


        DialogText*                 addTextScreen(const char*const text); //THESE NEED TO SET EACH SCREEN'S INDEX WHEN THEY'RE ADDED!!
        DialogPrompt*               addPromptScreen(const char*const text);

        unsigned                    getScreenCount(void) const;
        DialogScreen*               getScreen(const unsigned index) const;
        sol::object                  getScreenLua(const unsigned index) const;
        void                        setScreen(const unsigned index, DialogScreen*const screen);

        void                        setTriggerEntity(Entity*const entity);
        Entity*                     getTriggerEntity(void) const;
        int                         getStartScreen(void) const;
        int                         getStartScreenLua(void) const;
        void                        setStartScreen(const int index);
        void                        setStartScreenLua(const int index);
    };






    //==== DIALOGTEXT INLINEZ ====
    inline DialogTextBase::DialogTextBase(void):
        DialogScreen(DialogScreenBase::SCREEN_TEXT) {}
    inline DialogTextBase::DialogTextBase(const char*const text):
        DialogScreen(DialogScreenBase::SCREEN_TEXT, text) {}
    inline DialogTextBase::DialogTextBase(const DialogTextBase &rhs):
        LuaObject<DialogTextBase, DialogText, DialogScreen>(),
        DialogScreen(rhs) {
        *this = rhs;
    }
    inline void	DialogTextBase::setNextIndex(const int index) { _nextIndex = index; }
    inline int	DialogTextBase::getNextIndex(void) const { return _nextIndex; }
    inline void DialogTextBase::setNextIndexLua(const int index) { _nextIndex = (index == -1)? -1 : index-1; }
    inline int DialogTextBase::getNextIndexLua(void) const { return (_nextIndex == -1)? -1 :_nextIndex+1; }
    inline DialogTextBase& DialogTextBase::operator =(const DialogTextBase& rhs) {
        DialogScreen::operator=(rhs);
        _nextIndex = rhs._nextIndex;
        return *this;
    }

    //==== DIALOGOPTION INLINEZ ====
    inline DialogOptionBase::DialogOptionBase(const char *string, const int nextIndex):
        _nextIndex(nextIndex)
    {
        strcpy(_text, string);
    }
    inline DialogOptionBase::DialogOptionBase(const DialogOptionBase &rhs):
        LuaObject<DialogOptionBase, DialogOption>() {
        *this = rhs;
    }
    inline const char* DialogOptionBase::getText(void) const { return _text; }
    inline void DialogOptionBase::setText(const char *string) { strcpy(_text, string); }
    inline int DialogOptionBase::getNextIndex(void) const { return _nextIndex; }
    inline void DialogOptionBase::setNextIndex(const int index) { _nextIndex = index; }
    inline int DialogOptionBase::getNextIndexLua(void) const { return (_nextIndex == -1)? -1 : _nextIndex+1; }
    inline void DialogOptionBase::setNextIndexLua(const int index) { _nextIndex = (index == -1)? -1 : index-1; }
    inline DialogOptionBase& DialogOptionBase::operator =(const DialogOptionBase& rhs) {
        strcpy(_text, rhs._text);
        _nextIndex = rhs._nextIndex;
        return *this;
    }

    //==== DIALOGPROMPT INLINEZ =====
    inline DialogPromptBase::DialogPromptBase(void):
        DialogScreen(DialogScreenBase::SCREEN_PROMPT) {}
    inline DialogPromptBase::DialogPromptBase(const DialogPromptBase &rhs):
        LuaObject<DialogPromptBase, DialogPrompt, DialogScreen>(),
        DialogScreen(rhs) {
        *this = rhs;
    }
    inline DialogPromptBase::DialogPromptBase(const char*const text):
        DialogScreen(DialogScreenBase::SCREEN_PROMPT, text) {}
    inline unsigned DialogPromptBase::getOptionCount(void) const { return _options.size(); }



    //==== DIALOG INLINEZ =====

    inline DialogBase::DialogBase(const DialogBase &rhs):
        LuaObject<DialogBase, Dialog>() {
        *this = rhs;
    }
    inline int DialogBase::getStartScreen(void) const { return _startScreen; }
    inline int DialogBase::getStartScreenLua(void) const { return _startScreen+1; }
    inline void DialogBase::setStartScreen(const int index) { _startScreen = index; }
    inline void DialogBase::setStartScreenLua(const int index) { _startScreen = index-1; }
    inline unsigned DialogBase::getScreenCount(void) const { return _screens.size(); }
    inline DialogScreen* DialogBase::getScreen(const unsigned index) const { return index < _screens.size()? _screens[index] : nullptr; }
    inline void DialogBase::setTriggerEntity(Entity * const entity) { _triggerEntity = entity; }
    inline Entity* DialogBase::getTriggerEntity(void) const { return _triggerEntity; }







}

#endif
