#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/HDInstPage.hpp"
#include "HDInstall.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/color.hpp"
#include "util/lang.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;

    HDInstPage::HDInstPage() : Layout::Layout() {
        this->infoRect = Rectangle::New(0, 95, 1280, 60, TRANSPARENT_DARK);
        this->SetBackgroundColor(BLACK);
        this->topRect = Rectangle::New(0, 0, 1280, 94, TRANSPARENT_LIGHT);
        this->botRect = Rectangle::New(0, 659, 1280, 61, BLACK);
        this->SetBackgroundImage(inst::util::getBackground());
        this->logoImage = Image::New(20, 8, "romfs:/images/mapache-switch.png");
        this->titleImage = Image::New(160, 8, "romfs:/images/hdd.webp");
        this->appVersionText = TextBlock::New(1195, 60, "v" + inst::config::appVersion);
        this->appVersionText->SetColor(WHITE);
        this->pageInfoText = TextBlock::New(10, 109, "inst.hd.top_info"_lang);
        this->pageInfoText->SetColor(WHITE);
        this->butText = TextBlock::New(10, 678, "inst.hd.buttons"_lang);
        this->butText->SetColor(WHITE);
        this->menu = pu::ui::elm::Menu::New(0, 156, 1280, TRANSPARENT, 56, 9);
        this->menu->SetOnFocusColor(TRANSPARENT_LIGHTER);
        this->menu->SetScrollbarColor(TRANSPARENT_LIGHTER);
        this->Add(this->topRect);
        this->Add(this->infoRect);
        this->Add(this->botRect);
        this->Add(this->logoImage);
        this->Add(this->titleImage);
        this->Add(this->appVersionText);
        this->Add(this->butText);
        this->Add(this->pageInfoText);
        this->Add(this->menu);
    }

    void HDInstPage::drawMenuItems(bool clearItems, std::filesystem::path ourPath) {
        if (clearItems) this->selectedTitles = {};
		this->currentDir = ourPath;

		auto pathStr = this->currentDir.string();
		if(pathStr.length())
		{
			if(pathStr[pathStr.length() - 1] == ':')
			{
				this->currentDir = this->currentDir / "";
			}
		}

        this->menu->ClearItems();
        try {
            this->ourDirectories = util::getDirsAtPath(this->currentDir);
            this->ourFiles = util::getDirectoryFiles(this->currentDir, {".nsp", ".nsz", ".xci", ".xcz"});
        } catch (std::exception& e) {
            this->drawMenuItems(false, this->currentDir.parent_path());
            return;
        }

        std::string itm = "..";
        auto ourEntry = pu::ui::elm::MenuItem::New(itm);
        ourEntry->SetColor(WHITE);
        ourEntry->SetIcon("romfs:/images/icons/folder-upload.png");
        this->menu->AddItem(ourEntry);

        for (auto& file: this->ourDirectories) {
            if (file == "..") break;
            std::string itm = file.filename().string();
            auto ourEntry = pu::ui::elm::MenuItem::New(itm);
            ourEntry->SetColor(WHITE);
            ourEntry->SetIcon("romfs:/images/icons/folder.png");
            this->menu->AddItem(ourEntry);
        }
        for (auto& file: this->ourFiles) {
            std::string itm = file.filename().string();
            auto ourEntry = pu::ui::elm::MenuItem::New(itm);
            ourEntry->SetColor(WHITE);
            ourEntry->SetIcon("romfs:/images/icons/checkbox-blank-outline.png");
            for (long unsigned int i = 0; i < this->selectedTitles.size(); i++) {
                if (this->selectedTitles[i] == file) {
                    ourEntry->SetIcon("romfs:/images/icons/check-box-outline.png");
                }
            }
            this->menu->AddItem(ourEntry);
        }
    }

    void HDInstPage::followDirectory() {
        int selectedIndex = this->menu->GetSelectedIndex();
        int dirListSize = this->ourDirectories.size();

        dirListSize++;
        selectedIndex--;

        if (selectedIndex < dirListSize) {
            if (this->menu->GetItems()[this->menu->GetSelectedIndex()]->GetName() == ".." && this->menu->GetSelectedIndex() == 0) {
                this->drawMenuItems(true, this->currentDir.parent_path());
            } else {
                this->drawMenuItems(true, this->ourDirectories[selectedIndex]);
            }
            this->menu->SetSelectedIndex(0);
        }
    }

    void HDInstPage::selectNsp(int selectedIndex) {
        int dirListSize = this->ourDirectories.size();
        dirListSize++;

        if (this->menu->GetItems()[selectedIndex]->GetIcon() == "romfs:/images/icons/check-box-outline.png") {
            for (long unsigned int i = 0; i < this->selectedTitles.size(); i++) {
                if (this->selectedTitles[i] == this->ourFiles[selectedIndex - dirListSize]) this->selectedTitles.erase(this->selectedTitles.begin() + i);
            }
        } else if (this->menu->GetItems()[selectedIndex]->GetIcon() == "romfs:/images/icons/checkbox-blank-outline.png") this->selectedTitles.push_back(this->ourFiles[selectedIndex - dirListSize]);
        else {
            this->followDirectory();
            return;
        }
        this->drawMenuItems(false, currentDir);
    }

    void HDInstPage::startInstall() {
        int dialogResult = -1;
        std::vector<std::string> freeSpace = inst::util::mathstuff();
        std::string info = "space.SD.free"_lang + ": " + freeSpace[4] + " GB\n" + "space.system.free"_lang + ": " + freeSpace[1] + " GB\n\n";
        std::string dialogTitle;
        if (this->selectedTitles.size() == 1) {
            dialogTitle = "inst.target.desc0"_lang + inst::util::shortenString(std::filesystem::path(this->selectedTitles[0]).filename().string(), 32, true) + "inst.target.desc1"_lang;
        } else {
            dialogTitle = "inst.target.desc00"_lang + std::to_string(this->selectedTitles.size()) + "inst.target.desc01"_lang;
        };
        dialogResult = mainApp->CreateShowDialog(dialogTitle, info + "common.cancel_desc"_lang, {"inst.target.opt0"_lang, "inst.target.opt1"_lang}, false);
        if (dialogResult == -1) return;
        nspInstStuff_B::installNspFromFile(this->selectedTitles, dialogResult);
    }

    void HDInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & HidNpadButton_B) {
            mainApp->LoadLayout(mainApp->mainPage);
        }
        if ((Down & HidNpadButton_A) || (Up & TouchPseudoKey)) {
            this->selectNsp(this->menu->GetSelectedIndex());
            if (this->ourFiles.size() == 1 && this->selectedTitles.size() == 1) {
                this->startInstall();
            }
        }
        if ((Down & HidNpadButton_Y)) {
            if (this->selectedTitles.size() == this->ourFiles.size()) this->drawMenuItems(true, currentDir);
            else {
                int topDir = 0;
                topDir++;
                for (long unsigned int i = this->ourDirectories.size() + topDir; i < this->menu->GetItems().size(); i++) {
                    if (this->menu->GetItems()[i]->GetIcon() == "romfs:/images/icons/check-box-outline.png") continue;
                    else this->selectNsp(i);
                }
                this->drawMenuItems(false, currentDir);
            }
        }
        if ((Down & HidNpadButton_X)) {
            inst::ui::mainApp->CreateShowDialog("inst.hd.help.title"_lang, "inst.hd.help.desc"_lang, {"common.ok"_lang}, true);
        }
        if (Down & HidNpadButton_Plus) {
            if (this->selectedTitles.size() == 0 && this->menu->GetItems()[this->menu->GetSelectedIndex()]->GetIcon() == "romfs:/images/icons/checkbox-blank-outline.png") {
                this->selectNsp(this->menu->GetSelectedIndex());
            }
            if (this->selectedTitles.size() > 0) this->startInstall();
        }
    }
}
