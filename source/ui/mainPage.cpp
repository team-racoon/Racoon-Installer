#include <filesystem>
#include <iostream>
#include <sstream>
#include <switch.h>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/lang.hpp"
#include "sigInstall.hpp"
#include "data/buffered_placeholder_writer.hpp"
#include "nx/usbhdd.h"
#include <sys/statvfs.h>

#define COLOR(hex) pu::ui::Color::FromHex(hex)
	
int statvfs(const char *path, struct statvfs *buf);

double GetAvailableSpace(const char* path)
{
  struct statvfs stat;

  if (statvfs(path, &stat) != 0) {
    // error happens, just quits here
    return -1;
  }

  // the available size is f_bsize * f_bavail
  return stat.f_bsize * stat.f_bavail;
}

double amountOfDiskSpaceUsed(const char* path)
{
    struct statvfs stat;

    if (statvfs(path, &stat) != 0) {
      // error happens, just quits here
      return -1;
    }
    const auto total           = static_cast<unsigned long>(stat.f_blocks);
    const auto available       = static_cast<unsigned long>(stat.f_bavail);
    const auto availableToRoot = static_cast<unsigned long>(stat.f_bfree);
    const auto used            = total - availableToRoot;
    const auto nonRootTotal    = used + available;
    return 100.0 * static_cast<double>(used) / static_cast<double>(nonRootTotal);
}

double totalsize(const char* path)
{
    struct statvfs stat;

    if (statvfs(path, &stat) != 0) {
      // error happens, just quits here
      return -1;
    }
    return stat.f_blocks * stat.f_frsize;
}

namespace inst::ui {
    extern MainApplication *mainApp;
    bool appletFinished = false;
    bool updateFinished = false;

    void mathstuff() {
    	double math = (GetAvailableSpace("./") / 1024) / 1024; //megabytes
    	float math2 = ((float)math / 1024); //gigabytes
    	
    	double used = (amountOfDiskSpaceUsed("./")); //same file path as sdmc
    	
    	double total = (totalsize("sdmc:/") / 1024) / 1024; //megabytes
    	float total2 = ((float)total / 1024); //gigabytes
    	//
    	float GB = math2;
    	std::stringstream stream;
    	stream << std::fixed << std::setprecision(2) << GB; //only show 2 decimal places
    	std::string freespace = stream.str();
    		
    		
    	float GB2 = total2;
    	std::stringstream stream2;
    	stream2 << std::fixed << std::setprecision(2) << GB2; //only show 2 decimal places
    	std::string sdsize = stream2.str();
    	   		
    	//printf("\nSdCard Free Space in MB: %li", math);
    	//printf("\nSdCard Free Space in GB: %.2f", math2);
    	std::stringstream stream3;
    	stream3 << std::fixed << std::setprecision(2) << used; //only show 2 decimal places
    	std::string percent = stream3.str();
    		
    	//unmount sd here and mount system....
    	//fsdevUnmountDevice("sdmc");
    	FsFileSystem nandFS;
    	fsOpenBisFileSystem(&nandFS, FsBisPartitionId_User, "");
      fsdevMountDevice("user", nandFS);
      
      double math3 = (GetAvailableSpace("user:/") / 1024) / 1024; //megabytes
    	float math4 = ((float)math3 / 1024); //gigabytes
    	
    	double used2 = (amountOfDiskSpaceUsed("user:/")); //same file path as sdmc
    	
    	double total3 = (totalsize("user:/") / 1024) / 1024; //megabytes
    	float total4 = ((float)total3 / 1024); //gigabytes
    	//
    	float GB3 = math4;
    	std::stringstream stream4;
    	stream4 << std::fixed << std::setprecision(2) << GB3; //only show 2 decimal places
    	std::string freespace2 = stream4.str();
    		
    		
    	float GB4 = total4;
    	std::stringstream stream5;
    	stream5 << std::fixed << std::setprecision(2) << GB4; //only show 2 decimal places
    	std::string sdsize2 = stream5.str();
    	   		
    	//printf("\nSdCard Free Space in MB: %li", math);
    	//printf("\nSdCard Free Space in GB: %.2f", math2);
    	std::stringstream stream6;
    	stream6 << std::fixed << std::setprecision(2) << used2; //only show 2 decimal places
    	std::string percent2 = stream6.str();
    	
    	//unmount user now as we already know how much space we have	
    	fsdevUnmountDevice("user");
    	
    	
    	std::string Info = ("System total size: " + sdsize2 + " GB" + "\nSystem free space: " + freespace2 + " GB" + "\nSystem percent used: " + percent2 + "%" + "\n\n" + "SD card total size: " + sdsize + " GB" + "\nSD card free space: " + freespace + " GB" + "\nSD card percent used: " + percent + "%");
      inst::ui::mainApp->CreateShowDialog("Space Usage Information", Info, {"common.ok"_lang}, true);
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

    MainPage::MainPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#000000FF"));
        this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR("#000000FF"));
        this->botRect = Rectangle::New(0, 659, 1280, 61, COLOR("#000000FF"));
        
        if (inst::config::gayMode) {
        	if (std::filesystem::exists(inst::config::appDir + "/images/Main.png")) this->titleImage = Image::New(0, 0, (inst::config::appDir + "/images/Main.png"));
        	else 
        		this->titleImage = Image::New(0, 0, "romfs:/images/Main.png");
        	if (std::filesystem::exists(inst::config::appDir + "/images/Background.png")) this->SetBackgroundImage(inst::config::appDir + "/images/Background.png");
        	else
        		this->SetBackgroundImage("romfs:/images/Background.png");
        		//this->appVersionText = TextBlock::New(1240, 680, "v" + inst::config::appVersion, 20);
        }
        else {
        	this->SetBackgroundImage("romfs:/images/Background.png");
        	this->titleImage = Image::New(0, 0, "romfs:/images/Main.png");
        	//this->appVersionText = TextBlock::New(1200, 700, "v" + inst::config::appVersion, 10);
        }
        //this->appVersionText->SetColor(COLOR("#FFFFFFFF"));
        this->butText = TextBlock::New(10, 678, "main.buttons"_lang, 24);
        this->butText->SetColor(COLOR("#FFFFFFFF"));
        this->optionMenu = pu::ui::elm::Menu::New(0, 95, 1280, COLOR("#343E8700"), 94, 6);
        this->optionMenu->SetOnFocusColor(COLOR("#4f4f4d33"));
        this->optionMenu->SetScrollbarColor(COLOR("#1A1919FF"));
        this->installMenuItem = pu::ui::elm::MenuItem::New("main.menu.sd"_lang);
        this->installMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->installMenuItem->SetIcon("romfs:/images/icons/micro-sd.png");
        this->netInstallMenuItem = pu::ui::elm::MenuItem::New("main.menu.net"_lang);
        this->netInstallMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->netInstallMenuItem->SetIcon("romfs:/images/icons/cloud-download.png");
        this->usbInstallMenuItem = pu::ui::elm::MenuItem::New("main.menu.usb"_lang);
        this->usbInstallMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->usbInstallMenuItem->SetIcon("romfs:/images/icons/usb-port.png");
        this->sigPatchesMenuItem = pu::ui::elm::MenuItem::New("main.menu.hdd"_lang);
        this->sigPatchesMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->sigPatchesMenuItem->SetIcon("romfs:/images/icons/usb-hd.png");
        this->settingsMenuItem = pu::ui::elm::MenuItem::New("main.menu.set"_lang);
        this->settingsMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->settingsMenuItem->SetIcon("romfs:/images/icons/settings.png");
        this->exitMenuItem = pu::ui::elm::MenuItem::New("main.menu.exit"_lang);
        this->exitMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->exitMenuItem->SetIcon("romfs:/images/icons/exit-run.png");
        if (inst::config::gayMode) {
        	if (std::filesystem::exists(inst::config::appDir + "/images/Main.png")) this->awooImage = Image::New(0, 0, inst::config::appDir + "/images/Main.png");
        	else this->awooImage = Image::New(0, 0, "romfs:/images/Main.png");
      	}
      	else{
      		this->awooImage = Image::New(0, 0, "romfs:/images/Main.png");
      	}
        this->eggImage = Image::New(0, 0, "");
        this->Add(this->topRect);
        this->Add(this->botRect);
        this->Add(this->titleImage);
        //this->Add(this->appVersionText);
        this->Add(this->butText);
        this->optionMenu->AddItem(this->installMenuItem);
        this->optionMenu->AddItem(this->netInstallMenuItem);
        this->optionMenu->AddItem(this->usbInstallMenuItem);
        this->optionMenu->AddItem(this->sigPatchesMenuItem);
        this->optionMenu->AddItem(this->settingsMenuItem);
        this->optionMenu->AddItem(this->exitMenuItem);
        this->Add(this->awooImage);
        this->Add(this->eggImage);
        this->awooImage->SetVisible(!inst::config::gayMode);
        this->Add(this->optionMenu);
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

    void MainPage::sigPatchesMenuItem_Click() {
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
        if (((Down & HidNpadButton_Plus) || (Down & HidNpadButton_MINUS) || (Down & HidNpadButton_B)) && mainApp->IsShown()) {
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
                    MainPage::sigPatchesMenuItem_Click();
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
            this->awooImage->SetVisible(false);
            this->eggImage->SetVisible(true);
        }
        if (Up & HidNpadButton_X) {
            this->eggImage->SetVisible(false);
            if (!inst::config::gayMode) this->awooImage->SetVisible(true);
        }
        if (Down & HidNpadButton_Y) {
        		mathstuff();
        }
    }
}
