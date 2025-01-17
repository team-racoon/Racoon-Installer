#include <filesystem>
#include <switch.h>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/instPage.hpp"
#include "ui/optionsPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/color.hpp"
#include "util/error.hpp"
#include "util/curl.hpp"
#include "util/unzip.hpp"
#include "util/lang.hpp"
#include "ui/instPage.hpp"
#include "sigInstall.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;

    std::vector<std::string> languageStrings = {"English", "日本語", "Français", "Deutsch", "Italiano", "Русский"};

    optionsPage::optionsPage() : Layout::Layout() {
        this->infoRect = Rectangle::New(0, 95, 1280, 60, TRANSPARENT_DARK);
        this->SetBackgroundColor(BLACK);
        this->topRect = Rectangle::New(0, 0, 1280, 94, TRANSPARENT_LIGHT);
        this->botRect = Rectangle::New(0, 659, 1280, 61, BLACK);
        this->SetBackgroundImage(inst::util::getBackground());
        this->logoImage = Image::New(20, 8, "romfs:/images/mapache-switch.png");
        this->titleImage = Image::New(160, 5, "romfs:/images/settings.webp");
        this->appVersionText = TextBlock::New(1195, 60, "v" + inst::config::appVersion);
        this->appVersionText->SetColor(WHITE);
        this->pageInfoText = TextBlock::New(10, 109, "options.title"_lang);
        this->pageInfoText->SetColor(WHITE);
        this->butText = TextBlock::New(10, 678, "options.buttons"_lang);
        this->butText->SetColor(WHITE);
        this->menu = pu::ui::elm::Menu::New(0, 156, 1280, TRANSPARENT, 56, 9);
        this->menu->SetOnFocusColor(TRANSPARENT_LIGHTER);
        this->menu->SetScrollbarColor(TRANSPARENT_LIGHTER);
        this->Add(this->topRect);
        this->Add(this->infoRect);
        this->Add(this->logoImage);
        this->Add(this->titleImage);
        this->Add(this->appVersionText);
        this->Add(this->pageInfoText);
        this->setMenuText();
        this->Add(this->menu);
        this->Add(this->botRect);
        this->Add(this->butText);
    }

    void optionsPage::askToUpdate(std::vector<std::string> updateInfo) {
        if (!mainApp->CreateShowDialog("options.update.title"_lang, "options.update.desc0"_lang + updateInfo[0] + "options.update.desc1"_lang, {"options.update.opt0"_lang, "common.cancel"_lang}, false)) {
            inst::ui::instPage::loadInstallScreen();
            inst::ui::instPage::setTopInstInfoText("options.update.top_info"_lang + updateInfo[0]);
            inst::ui::instPage::setInstBarPerc(0);
            inst::ui::instPage::setInstInfoText("options.update.bot_info"_lang + updateInfo[0]);
            try {
                std::string downloadName = inst::config::appDir + "/temp_download.zip";
                inst::curl::downloadFile(updateInfo[1], downloadName.c_str(), 0, true);
                romfsExit();
                inst::ui::instPage::setInstInfoText("options.update.bot_info2"_lang + updateInfo[0]);
                inst::zip::extractFile(downloadName, "sdmc:/");
                std::filesystem::remove(downloadName);
                mainApp->CreateShowDialog("options.update.complete"_lang, "options.update.end_desc"_lang, {"common.ok"_lang}, false);
            } catch (...) {
                mainApp->CreateShowDialog("options.update.failed"_lang, "options.update.end_desc"_lang, {"common.ok"_lang}, false);
            }
            mainApp->FadeOut();
            mainApp->Close();
        }
        return;
    }

    std::string optionsPage::getMenuOptionIcon(bool ourBool) {
        if(ourBool) return "romfs:/images/icons/check-box-outline.png";
        else return "romfs:/images/icons/checkbox-blank-outline.png";
    }

    std::string optionsPage::getMenuLanguage(int ourLangCode) {
        switch (ourLangCode) {
            case 1:
            case 12:
                return languageStrings[0];
            case 0:
                return languageStrings[1];
            case 2:
            case 13:
                return languageStrings[2];
            case 3:
                return languageStrings[3];
            case 4:
                return languageStrings[4];
            case 10:
                return languageStrings[5];
            default:
                return "options.language.system_language"_lang;
        }
    }
    
    void sigPatchesMenuItem_Click() {
        sig::installSigPatches();
    }
    
    void optionsPage::setMenuText() {
        this->menu->ClearItems();
        auto ignoreFirmOption = pu::ui::elm::MenuItem::New("options.menu_items.ignore_firm"_lang);
        ignoreFirmOption->SetColor(WHITE);
        ignoreFirmOption->SetIcon(this->getMenuOptionIcon(inst::config::ignoreReqVers));
        this->menu->AddItem(ignoreFirmOption);
        auto validateOption = pu::ui::elm::MenuItem::New("options.menu_items.nca_verify"_lang);
        validateOption->SetColor(WHITE);
        validateOption->SetIcon(this->getMenuOptionIcon(inst::config::validateNCAs));
        this->menu->AddItem(validateOption);
        auto overclockOption = pu::ui::elm::MenuItem::New("options.menu_items.boost_mode"_lang);
        overclockOption->SetColor(WHITE);
        overclockOption->SetIcon(this->getMenuOptionIcon(inst::config::overClock));
        this->menu->AddItem(overclockOption);
        auto deletePromptOption = pu::ui::elm::MenuItem::New("options.menu_items.ask_delete"_lang);
        deletePromptOption->SetColor(WHITE);
        deletePromptOption->SetIcon(this->getMenuOptionIcon(inst::config::deletePrompt));
        this->menu->AddItem(deletePromptOption);
        auto autoUpdateOption = pu::ui::elm::MenuItem::New("options.menu_items.auto_update"_lang);
        autoUpdateOption->SetColor(WHITE);
        autoUpdateOption->SetIcon(this->getMenuOptionIcon(inst::config::autoUpdate));
        this->menu->AddItem(autoUpdateOption);
              
        auto useSoundOption = pu::ui::elm::MenuItem::New("options.menu_items.useSound"_lang);
        useSoundOption->SetColor(WHITE);
        useSoundOption->SetIcon(this->getMenuOptionIcon(inst::config::useSound));
        this->menu->AddItem(useSoundOption);
        
        auto SigPatch = pu::ui::elm::MenuItem::New("main.menu.sig"_lang);
        SigPatch->SetColor(WHITE);
        this->menu->AddItem(SigPatch);
        
        // auto sigPatchesUrlOption = pu::ui::elm::MenuItem::New("options.menu_items.sig_url"_lang + inst::util::shortenString(inst::config::sigPatchesUrl, 42, false));
        // sigPatchesUrlOption->SetColor(WHITE);
        // this->menu->AddItem(sigPatchesUrlOption);
        auto languageOption = pu::ui::elm::MenuItem::New("options.menu_items.language"_lang + this->getMenuLanguage(inst::config::languageSetting));
        languageOption->SetColor(WHITE);
        this->menu->AddItem(languageOption);
        auto updateOption = pu::ui::elm::MenuItem::New("options.menu_items.check_update"_lang);
        updateOption->SetColor(WHITE);
        this->menu->AddItem(updateOption);
        auto creditsOption = pu::ui::elm::MenuItem::New("options.menu_items.credits"_lang);
        creditsOption->SetColor(WHITE);
        this->menu->AddItem(creditsOption);
    }

    void optionsPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & HidNpadButton_B) {
            mainApp->LoadLayout(mainApp->mainPage);
        }
        if ((Down & HidNpadButton_A) || (Up & TouchPseudoKey)) {
            std::string keyboardResult;
            int rc;
            std::vector<std::string> downloadUrl;
            std::vector<pu::String> languageList;
            switch (this->menu->GetSelectedIndex()) {
                case 0:
                    inst::config::ignoreReqVers = !inst::config::ignoreReqVers;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 1:
                    if (inst::config::validateNCAs) {
                        if (inst::ui::mainApp->CreateShowDialog("options.nca_warn.title"_lang, "options.nca_warn.desc"_lang, {"common.cancel"_lang, "options.nca_warn.opt1"_lang}, false) == 1) inst::config::validateNCAs = false;
                    } else inst::config::validateNCAs = true;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 2:
                    inst::config::overClock = !inst::config::overClock;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 3:
                    inst::config::deletePrompt = !inst::config::deletePrompt;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 4:
                    inst::config::autoUpdate = !inst::config::autoUpdate;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 5:
                    if (inst::config::useSound) {
                        inst::config::useSound = false;
                    }
                    else {
                        inst::config::useSound = true;
                    }
                    this->setMenuText();
                    inst::config::setConfig();
                    break;
                
                case 6:
                    sigPatchesMenuItem_Click();
                    break;
                // case 8:
                //     keyboardResult = inst::util::softwareKeyboard("options.sig_hint"_lang, inst::config::sigPatchesUrl.c_str(), 500);
                //     if (keyboardResult.size() > 0) {
                //         inst::config::sigPatchesUrl = keyboardResult;
                //         inst::config::setConfig();
                //         this->setMenuText();
                //     }
                //     break;
                case 7:
                    for (size_t i = 0; i < languageStrings.size(); i++)
                    {
                        languageList.push_back(pu::String(languageStrings[i]));
                    };
                    languageList.push_back(pu::String("options.language.system_language"_lang));
                    rc = inst::ui::mainApp->CreateShowDialog("options.language.title"_lang, "options.language.desc"_lang, languageList, false);
                    if (rc == -1) break;
                    switch(rc) {
                        case 0:
                            inst::config::languageSetting = 1;
                            break;
                        case 1:
                            inst::config::languageSetting = 0;
                            break;
                        case 2:
                            inst::config::languageSetting = 2;
                            break;
                        case 3:
                            inst::config::languageSetting = 3;
                            break;
                        case 4:
                            inst::config::languageSetting = 4;
                            break;
                        case 5:
                            inst::config::languageSetting = 10;
                            break;
                        default:
                            inst::config::languageSetting = 99;
                    }
                    inst::config::setConfig();
                    mainApp->FadeOut();
                    mainApp->Close();
                    break;
                case 8:
                    if (inst::util::getIPAddress() == "1.0.0.127") {
                        inst::ui::mainApp->CreateShowDialog("main.net.title"_lang, "main.net.desc"_lang, {"common.ok"_lang}, true);
                        break;
                    }
                    downloadUrl = inst::util::checkForAppUpdate();
                    if (!downloadUrl.size()) {
                        mainApp->CreateShowDialog("options.update.title_check_fail"_lang, "options.update.desc_check_fail"_lang, {"common.ok"_lang}, false);
                        break;
                    }
                    this->askToUpdate(downloadUrl);
                    break;
                case 9:
                    inst::ui::mainApp->CreateShowDialog("options.credits.title"_lang, "options.credits.desc"_lang, {"common.close"_lang}, true);
                    break;
                default:
                    break;
            }
        }
    }
}
