#include <filesystem>
#include <iostream>
#include <sstream>
#include <switch.h>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/color.hpp"
#include "util/lang.hpp"
#include "sigInstall.hpp"
#include "data/buffered_placeholder_writer.hpp"
#include "nx/usbhdd.h"

namespace inst::ui {
    extern MainApplication *mainApp;
    bool appletFinished = false;
    bool updateFinished = false;

    void showFreeSpace(){
        std::vector<std::string> values = inst::util::mathstuff();
        std::string sdsize2 = values[0];
        std::string freespace2 = values[1];
        std::string percent2 = values[2];
        std::string sdsize = values[3];
        std::string freespace = values[4];
        std::string percent = values[5];
        std::string Info = ("space.system.size"_lang + ": " + sdsize2 + " GB" + "\n"+"space.system.free"_lang + ": " + freespace2 + " GB" + "\n"+"space.system.percent"_lang + ": " + percent2 + "%" + "\n\n" + "space.SD.size"_lang + ": " + sdsize + " GB" + "\n"+"space.SD.free"_lang + ": " + freespace + " GB" + "\n"+"space.SD.percent"_lang + ": " + percent + "%");
        inst::ui::mainApp->CreateShowDialog("space.title"_lang, Info, {"common.ok"_lang}, true);
    }
    
    void mainMenuThread() {
        bool menuLoaded = mainApp->IsShown();
        if (!appletFinished && appletGetAppletType() == AppletType_LibraryApplet) {
            tin::data::NUM_BUFFER_SEGMENTS = 2;
            if (menuLoaded) {
                inst::ui::appletFinished = true;
                mainApp->CreateShowDialog("main.applet.title"_lang, "main.applet.desc"_lang, {"common.ok"_lang}, true);
            } 
        } else if (!appletFinished) {
            inst::ui::appletFinished = true;
            tin::data::NUM_BUFFER_SEGMENTS = 128;
        }
        if (!updateFinished && (!inst::config::autoUpdate || inst::util::getIPAddress() == "1.0.0.127")) updateFinished = true;
        
        if (!updateFinished && menuLoaded && inst::config::updateInfo.size()) {
            updateFinished = true;
            optionsPage::askToUpdate(inst::config::updateInfo);
        }
    }

    Image::Ref getRacoonImage() {
        if( appletGetAppletType() == AppletType_LibraryApplet) {
            return Image::New( 630, 89, "romfs:/images/applet.webp");
        } return Image::New( 690, 200, "romfs:/images/override.webp");
    }

    MainPage::MainPage() : Layout::Layout() {
        this->SetBackgroundColor(BLACK);
        this->topRect = Rectangle::New(0, 0, 1280, 94, TRANSPARENT_LIGHT);
        this->botRect = Rectangle::New(0, 659, 1280, 61, BLACK);
        this->SetBackgroundImage(inst::util::getBackground());
        this->logoImage = Image::New(20, 8, "romfs:/images/mapache-switch.png");
        this->appVersionText = TextBlock::New(1195, 60, "v" + inst::config::appVersion);
        this->appVersionText->SetColor(WHITE);
        this->butText = TextBlock::New(10, 678, "main.buttons"_lang);
        this->butText->SetColor(WHITE);
        this->optionMenu = pu::ui::elm::Menu::New(0, 95, 1280, TRANSPARENT, 94, 6);
        this->optionMenu->SetOnFocusColor(TRANSPARENT_LIGHTER);
        this->optionMenu->SetScrollbarColor(TRANSPARENT_LIGHTER);
        this->installMenuItem = pu::ui::elm::MenuItem::New("main.menu.sd"_lang);
        this->installMenuItem->SetColor(WHITE);
        this->installMenuItem->SetIcon("romfs:/images/icons/microsd-icon.png");
        this->netInstallMenuItem = pu::ui::elm::MenuItem::New("main.menu.net"_lang);
        this->netInstallMenuItem->SetColor(WHITE);
        this->netInstallMenuItem->SetIcon("romfs:/images/icons/cloud-icon.png");
        this->usbInstallMenuItem = pu::ui::elm::MenuItem::New("main.menu.usb"_lang);
        this->usbInstallMenuItem->SetColor(WHITE);
        this->usbInstallMenuItem->SetIcon("romfs:/images/icons/usb-icon.png");
        this->hddInstallMenuItem = pu::ui::elm::MenuItem::New("main.menu.hdd"_lang);
        this->hddInstallMenuItem->SetColor(WHITE);
        this->hddInstallMenuItem->SetIcon("romfs:/images/icons/hdd-icon.png");
        this->settingsMenuItem = pu::ui::elm::MenuItem::New("main.menu.set"_lang);
        this->settingsMenuItem->SetColor(WHITE);
        this->settingsMenuItem->SetIcon("romfs:/images/icons/settings-icon.png");
        this->exitMenuItem = pu::ui::elm::MenuItem::New("main.menu.exit"_lang);
        this->exitMenuItem->SetColor(WHITE);
        this->exitMenuItem->SetIcon("romfs:/images/icons/exit-icon.png");
        this->titleImage = Image::New(160, 8, "romfs:/images/title.webp");
        this->racoonImage = getRacoonImage();
        this->Add(this->topRect);
        this->Add(this->logoImage);
        this->optionMenu->AddItem(this->installMenuItem);
        this->optionMenu->AddItem(this->netInstallMenuItem);
        this->optionMenu->AddItem(this->usbInstallMenuItem);
        this->optionMenu->AddItem(this->hddInstallMenuItem);
        this->optionMenu->AddItem(this->settingsMenuItem);
        this->optionMenu->AddItem(this->exitMenuItem);
        this->Add(this->optionMenu);
        this->Add(this->botRect);
        this->Add(this->butText);
        this->Add(this->titleImage);
        this->Add(this->racoonImage);
        this->Add(this->appVersionText);
        this->AddThread(mainMenuThread);
    }

    void MainPage::installMenuItem_Click() {
        mainApp->sdinstPage->drawMenuItems(true, "sdmc:/");
        mainApp->sdinstPage->menu->SetSelectedIndex(0);
        mainApp->LoadLayout(mainApp->sdinstPage);
    }

    void MainPage::netInstallMenuItem_Click() {
        if (inst::util::getIPAddress() == "1.0.0.127") {
            inst::ui::mainApp->CreateShowDialog("main.net.title"_lang, "main.net.desc"_lang, {"common.ok"_lang}, true);
            return;
        }
        mainApp->netinstPage->startNetwork();
    }

    void MainPage::usbInstallMenuItem_Click() {
        if (!inst::config::usbAck) {
            if (mainApp->CreateShowDialog("main.usb.warn.title"_lang, "main.usb.warn.desc"_lang, {"common.ok"_lang, "main.usb.warn.opt1"_lang}, false) == 1) {
                inst::config::usbAck = true;
                inst::config::setConfig();
            }
        }
        if (inst::util::getUsbState() == 5) mainApp->usbinstPage->startUsb();
        else mainApp->CreateShowDialog("main.usb.error.title"_lang, "main.usb.error.desc"_lang, {"common.ok"_lang}, false);
    }

    void MainPage::hddInstallMenuItem_Click() {
		if(nx::hdd::count() && nx::hdd::rootPath()) {
			mainApp->HDinstPage->drawMenuItems(true, nx::hdd::rootPath());
			mainApp->HDinstPage->menu->SetSelectedIndex(0);
			mainApp->LoadLayout(mainApp->HDinstPage);
		} else {
			inst::ui::mainApp->CreateShowDialog("main.hdd.title"_lang, "main.hdd.notfound"_lang, {"common.ok"_lang}, true);
		}
    }

    void MainPage::exitMenuItem_Click() {
        mainApp->FadeOut();
        mainApp->Close();
    }

    void MainPage::settingsMenuItem_Click() {
        mainApp->LoadLayout(mainApp->optionspage);
    }

    void MainPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (((Down & HidNpadButton_Plus) || (Down & HidNpadButton_Minus) || (Down & HidNpadButton_B)) && mainApp->IsShown()) {
            mainApp->FadeOut();
            mainApp->Close();
        }
        if ((Down & HidNpadButton_A) || (Up & TouchPseudoKey)) {
            switch (this->optionMenu->GetSelectedIndex()) {
                case 0:
                    this->installMenuItem_Click();
                    break;
                case 1:
                    this->netInstallMenuItem_Click();
                    break;
                case 2:
                    MainPage::usbInstallMenuItem_Click();
                    break;
                case 3:
                    MainPage::hddInstallMenuItem_Click();
                    break;
                case 4:
                    MainPage::settingsMenuItem_Click();
                    break;
                case 5:
                    MainPage::exitMenuItem_Click();
                    break;
                default:
                    break;
            }
        }
        
        if (Down & HidNpadButton_X) {
            // this->logoImage->SetVisible(false);
            // this->racoonImage->SetVisible(true);
        }
        if (Up & HidNpadButton_X) {
            // this->racoonImage->SetVisible(false);
        }
        if (Down & HidNpadButton_Y) {
        		showFreeSpace();
        }
    }
}
