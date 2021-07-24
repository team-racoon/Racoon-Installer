#include "ui/usbInstPage.hpp"
#include "ui/MainApplication.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/color.hpp"
#include "util/lang.hpp"
#include "usbInstall.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;

    usbInstPage::usbInstPage() : Layout::Layout() {
        this->infoRect = Rectangle::New(0, 95, 1280, 60, TRANSPARENT_DARK);
        this->SetBackgroundColor(BLACK);
        this->topRect = Rectangle::New(0, 0, 1280, 94, TRANSPARENT_LIGHT);
        this->botRect = Rectangle::New(0, 659, 1280, 61, BLACK);
        this->SetBackgroundImage(inst::util::getBackground());
        this->logoImage = Image::New(20, 8, "romfs:/images/mapache-switch.png");
        this->titleImage = Image::New(160, 8, "romfs:/images/usb.webp");
        this->appVersionText = TextBlock::New(1195, 60, "v" + inst::config::appVersion);
        this->appVersionText->SetColor(WHITE);
        this->pageInfoText = TextBlock::New(10, 109, "");
        this->pageInfoText->SetColor(WHITE);
        this->butText = TextBlock::New(10, 678, "");
        this->butText->SetColor(WHITE);
        this->menu = pu::ui::elm::Menu::New(0, 156, 1280, TRANSPARENT, 56, 9);
        this->menu->SetOnFocusColor(TRANSPARENT_LIGHTER);
        this->menu->SetScrollbarColor(TRANSPARENT_LIGHTER);
        this->infoImage = Image::New(460, 332, "romfs:/images/icons/usb-connection-waiting.png");
        this->Add(this->topRect);
        this->Add(this->infoRect);
        this->Add(this->logoImage);
        this->Add(this->titleImage);
        this->Add(this->appVersionText);
        this->Add(this->pageInfoText);
        this->Add(this->menu);
        this->Add(this->botRect);
        this->Add(this->butText);
        this->Add(this->infoImage);
    }

    void usbInstPage::drawMenuItems(bool clearItems) {
        if (clearItems) this->selectedTitles = {};
        this->menu->ClearItems();
        for (auto& url: this->ourTitles) {
            std::string itm = inst::util::shortenString(inst::util::formatUrlString(url), 56, true);
            auto ourEntry = pu::ui::elm::MenuItem::New(itm);
            ourEntry->SetColor(WHITE);
            ourEntry->SetIcon("romfs:/images/icons/checkbox-blank-outline.png");
            for (long unsigned int i = 0; i < this->selectedTitles.size(); i++) {
                if (this->selectedTitles[i] == url) {
                    ourEntry->SetIcon("romfs:/images/icons/check-box-outline.png");
                }
            }
            this->menu->AddItem(ourEntry);
        }
    }

    void usbInstPage::selectTitle(int selectedIndex) {
        if (this->menu->GetItems()[selectedIndex]->GetIcon() == "romfs:/images/icons/check-box-outline.png") {
            for (long unsigned int i = 0; i < this->selectedTitles.size(); i++) {
                if (this->selectedTitles[i] == this->ourTitles[selectedIndex]) this->selectedTitles.erase(this->selectedTitles.begin() + i);
            }
        } else this->selectedTitles.push_back(this->ourTitles[selectedIndex]);
        this->drawMenuItems(false);
    }

    void usbInstPage::startUsb() {
        this->pageInfoText->SetText("inst.usb.top_info"_lang);
        this->butText->SetText("inst.usb.buttons"_lang);
        this->menu->SetVisible(false);
        this->menu->ClearItems();
        this->infoImage->SetVisible(true);
        mainApp->LoadLayout(mainApp->usbinstPage);
        mainApp->CallForRender();
        this->ourTitles = usbInstStuff::OnSelected();
        if (!this->ourTitles.size()) {
            mainApp->LoadLayout(mainApp->mainPage);
            return;
        } else {
            mainApp->CallForRender(); // If we re-render a few times during this process the main screen won't flicker
            this->pageInfoText->SetText("inst.usb.top_info2"_lang);
            this->butText->SetText("inst.usb.buttons2"_lang);
            this->drawMenuItems(true);
            this->menu->SetSelectedIndex(0);
            mainApp->CallForRender();
            this->infoImage->SetVisible(false);
            this->menu->SetVisible(true);
        }
        return;
    }

    void usbInstPage::startInstall() {
        int dialogResult = -1;
        std::vector<std::string> freeSpace = inst::util::mathstuff();
        std::string info = "space.SD.free"_lang + ": " + freeSpace[4] + " GB\n" + "space.system.free"_lang + ": " + freeSpace[1] + " GB\n\n";
        std::string dialogTitle;
        if (this->selectedTitles.size() == 1){
             dialogTitle = "inst.target.desc0"_lang  + inst::util::shortenString(inst::util::formatUrlString(this->selectedTitles[0]), 32, true) + "inst.target.desc1"_lang;
        } else {
            dialogTitle = "inst.target.desc00"_lang + std::to_string(this->selectedTitles.size()) + "inst.target.desc01"_lang;
        }
        dialogResult = mainApp->CreateShowDialog(dialogTitle, info + "common.cancel_desc"_lang, {"inst.target.opt0"_lang, "inst.target.opt1"_lang}, false);
        if (dialogResult == -1) return;
        usbInstStuff::installTitleUsb(this->selectedTitles, dialogResult);
        return;
    }

    void usbInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & HidNpadButton_B) {
            mainApp->LoadLayout(mainApp->mainPage);
        }
        if ((Down & HidNpadButton_A) || (Up & TouchPseudoKey)) {
            this->selectTitle(this->menu->GetSelectedIndex());
            if (this->menu->GetItems().size() == 1 && this->selectedTitles.size() == 1) {
                this->startInstall();
            }
        }
        if ((Down & HidNpadButton_Y)) {
            if (this->selectedTitles.size() == this->menu->GetItems().size()) this->drawMenuItems(true);
            else {
                for (long unsigned int i = 0; i < this->menu->GetItems().size(); i++) {
                    if (this->menu->GetItems()[i]->GetIcon() == "romfs:/images/icons/check-box-outline.png") continue;
                    else this->selectTitle(i);
                }
                this->drawMenuItems(false);
            }
        }
        if (Down & HidNpadButton_Plus) {
            if (this->selectedTitles.size() == 0) {
                this->selectTitle(this->menu->GetSelectedIndex());
                this->startInstall();
                return;
            }
            this->startInstall();
        }
    }
}
