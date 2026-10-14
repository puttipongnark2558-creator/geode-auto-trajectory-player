#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

static bool g_isAutoPlaying = false;
static bool g_shouldHoldJump = false;

class $modify(AutoPlayLayer, PlayLayer) {
    struct Fields {
        bool m_safeModeActive = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        g_isAutoPlaying = Mod::get()->getSettingValue<bool>("auto-play-enabled");
        m_fields->m_safeModeActive = Mod::get()->getSettingValue<bool>("auto-safe-mode");

        if (g_isAutoPlaying) {
            Notification::create("Auto Player Active (All Modes Support)", NotificationIcon::Info)->show();
        }

        return true;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (!g_isAutoPlaying || !m_player1 || m_isDead) return;

        bool shouldClick = false;
        auto player = m_player1;

        if (player->m_isShip || player->m_isBird || player->m_isSwing) {
            shouldClick = (player->getPositionY() < 150.f);
        } else if (player->m_isDart) {
            shouldClick = (int(player->getPositionX()) % 20 < 10);
        } else {
            shouldClick = player->m_isOnGround;
        }

        if (shouldClick && !g_shouldHoldJump) {
            g_shouldHoldJump = true;
            this->handleButton(true, (int)PlayerButton::Jump, true);
        } else if (!shouldClick && g_shouldHoldJump) {
            g_shouldHoldJump = false;
            this->handleButton(false, (int)PlayerButton::Jump, true);
        }
    }

    void levelComplete() {
        if (g_isAutoPlaying || m_fields->m_safeModeActive) {
            Notification::create("Safe Mode: Level completion ignored & stats protected", NotificationIcon::Warning)->show();
            this->onQuit();
            return;
        }
        PlayLayer::levelComplete();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);
        if (g_isAutoPlaying && Mod::get()->getSettingValue<bool>("auto-retry")) {
            this->resetLevel();
        }
    }
};

class $modify(AutoPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto menu = this->getChildByID("left-button-menu");
        if (!menu) menu = this->getChildByID("center-button-menu");
        if (menu) {
            auto btnSprite = ButtonSprite::create("Auto Bot", "goldFont.fnt", "GJ_button_01.png", 0.8f);
            auto btn = CCMenuItemSpriteExtra::create(
                btnSprite,
                this,
                menu_selector(AutoPauseLayer::onToggleAutoPlay)
            );
            btn->setID("auto-play-toggle-btn"_spr);
            menu->addChild(btn);
            menu->updateLayout();
        }
    }

    void onToggleAutoPlay(CCObject* sender) {
        g_isAutoPlaying = !g_isAutoPlaying;
        Mod::get()->setSettingValue("auto-play-enabled", g_isAutoPlaying);
        std::string msg = g_isAutoPlaying ? "Autoplay Enabled (Safe Mode Active)" : "Autoplay Disabled";
        Notification::create(msg, NotificationIcon::Info)->show();
    }
};
