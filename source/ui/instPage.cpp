#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/instPage.hpp"
#include "util/config.hpp"
#include "util/color.hpp"
#include "util/util.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;

    instPage::instPage() : Layout::Layout() {    	
        this->infoRect = Rectangle::New(0, 95, 1280, 60, TRANSPARENT_DARK);
        this->SetBackgroundColor(BLACK);
        this->topRect = Rectangle::New(0, 0, 1280, 94, TRANSPARENT_LIGHT);
        this->SetBackgroundImage(inst::util::getBackground());
        this->logoImage = Image::New(20, 8, "romfs:/images/mapache-switch.png");
        this->titleImage = Image::New(160, 8, "romfs:/images/install.webp");
        this->appVersionText = TextBlock::New(1195, 60, "v" + inst::config::appVersion);
        this->appVersionText->SetColor(WHITE);
        this->pageInfoText = TextBlock::New(10, 109, "");
        this->pageInfoText->SetColor(WHITE);
        this->installInfoText = TextBlock::New(15, 645, "");
        this->installInfoText->SetColor(WHITE);
        this->installBar = pu::ui::elm::ProgressBar::New(10, 685, 1260, 20, 100.0f);
        this->installBar->SetColor(BLACK);
        this->installBar->SetProgressColor(COLOR("#56FF59FF"));
        this->Add(this->topRect);
        this->Add(this->infoRect);
        this->Add(this->logoImage);
        this->Add(this->titleImage);
        this->Add(this->appVersionText);
        this->Add(this->pageInfoText);
        this->Add(this->installInfoText);
        this->Add(this->installBar);
    }

    void instPage::setTopInstInfoText(std::string ourText){
        mainApp->instpage->pageInfoText->SetText(ourText);
        mainApp->CallForRender();
    }

    void instPage::setInstInfoText(std::string ourText){
        mainApp->instpage->installInfoText->SetText(ourText);
        mainApp->CallForRender();
    }

    void instPage::setInstBarPerc(double ourPercent){
        mainApp->instpage->installBar->SetVisible(true);
        mainApp->instpage->installBar->SetProgress(ourPercent);
        mainApp->CallForRender();
    }

    void instPage::loadMainMenu(){
        mainApp->LoadLayout(mainApp->mainPage);
    }

    void instPage::loadInstallScreen(){
        mainApp->instpage->pageInfoText->SetText("");
        mainApp->instpage->installInfoText->SetText("");
        mainApp->instpage->installBar->SetProgress(0);
        mainApp->instpage->installBar->SetVisible(false);
        mainApp->LoadLayout(mainApp->instpage);
        mainApp->CallForRender();
    }

    void instPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
    }
}
