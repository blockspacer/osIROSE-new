#include "entitySystem.h"
#include "systems/movementsystem.h"
#include "systems/updatesystem.h"
#include "systems/chatsystem.h"
#include "systems/inventorysystem.h"
#include "systems/partysystem.h"
#include "database.h"

using namespace RoseCommon;

EntitySystem::EntitySystem() : systemManager_(*this) {
    systemManager_.add<Systems::MovementSystem>();
    systemManager_.add<Systems::UpdateSystem>();
    systemManager_.add<Systems::ChatSystem>();
    systemManager_.add<Systems::InventorySystem>();
    systemManager_.add<Systems::PartySystem>();
}

EntityManager &EntitySystem::getEntityManager() {
    return entityManager_;
}

void EntitySystem::registerEntity(Entity entity) {
    if (!entity)
        return;
    auto basic = entity.component<BasicInfo>();
    if (!basic || basic->name_ == "" || !basic->id_)
        return;
    nameToEntity_[basic->name_] = entity;
    idToEntity_[basic->id_] = entity;
}

Entity EntitySystem::getEntity(const std::string &name) {
    return nameToEntity_[name];
}

Entity EntitySystem::getEntity(uint32_t charId) {
    return idToEntity_[charId];
}

void EntitySystem::update(double dt) {
    systemManager_.update(dt);
    for (auto it : toDestroy_) {
        if (it)
            it.destroy();
    }
    toDestroy_.clear();
}

void EntitySystem::destroy(Entity entity) {
    if (!entity)
        return;
    if (entity.component<SocketConnector>())
        entity.remove<SocketConnector>();
    toDestroy_.push_back(entity);
}

Entity EntitySystem::create() {
    return entityManager_.create();
}

bool EntitySystem::isNearby(Entity a, Entity b) {
    return true; // FIXME : actually implement the sight calculation instead of the distance
    if (!a || !b)
        return false;
    auto posa = a.component<Position>();
    auto posb = b.component<Position>();
    if (!posa || !posb)
        return false; // FIXME : is it a bug if there is no position?
    if (posa->map_ != posb->map_)
        return false;
    double dist = (posa->x_ - posb->x_) * (posa->x_ - posb->x_) + (posa->y_ - posb->y_) * (posa->y_ - posb->y_);
    if (dist > NEARBY_DIST)
        return false;
    return true;
}

bool EntitySystem::dispatch(Entity entity, const RoseCommon::CRosePacket &packet) {
    if (!entity)
        return false;
    return systemManager_.dispatch(entity, packet);
}

Entity EntitySystem::loadCharacter(uint32_t charId, bool platinium, uint32_t id) {
    auto &database = Core::databasePool.getDatabase();
    auto res = database.QStore(fmt::format("CALL get_character({});", charId));
    auto entity = create();
    if (!res || res->size() != 1) {
        entity.destroy();
        return Entity();
    }
    auto pos = entity.assign<Position>();
    res->getFloat("x", pos->x_);
    res->getFloat("y", pos->y_);
    res->getInt("map", pos->map_);
    res->getInt("revive_map", pos->spawn_);

    auto basic = entity.assign<BasicInfo>();
    res->getString("name", basic->name_);
    res->getInt("exp", basic->xp_);
    res->getInt("level", basic->level_);
    basic->id_ = id;
    basic->tag_ = id;

    auto stats = entity.assign<Stats>();
    res->getInt("max_hp", stats->maxHp_);
    res->getInt("max_mp", stats->maxMp_);
    res->getInt("str", stats->str_);
    res->getInt("dex", stats->dex_);
    res->getInt("int_", stats->int_);
    res->getInt("con", stats->con_);
    res->getInt("charm", stats->charm_);
    res->getInt("sense", stats->sense_);

    auto advanced = entity.assign<AdvancedInfo>();
    res->getInt("zuly", advanced->zuly_);
    res->getInt("current_hp", advanced->hp_);
    res->getInt("current_mp", advanced->mp_);

    auto graphics = entity.assign<CharacterGraphics>();
    res->getInt("face", graphics->face_);
    res->getInt("hair", graphics->hair_);
    res->getInt("race", graphics->race_);

    auto info = entity.assign<CharacterInfo>();
    res->getInt("job", info->job_);
    res->getInt("stone", info->stone_);
    res->getInt("stat_points", info->statPoints_);
    res->getInt("skill_points", info->skillPoints_);
    res->getInt("penalty_exp", info->penaltyXp_);
    res->getInt("delete_date", info->deleteDate_);
    info->platinium_ = platinium;
    res->getInt("factionid", info->factionId_);
    res->getInt("faction_rank", info->factionRank_);
    res->getInt("fame", info->fame_);
    res->getInt("faction_fame1", info->factionFame_[0]);
    res->getInt("faction_fame2", info->factionFame_[1]);
    res->getInt("faction_points1", info->factionPoints_[0]);
    res->getInt("faction_points2", info->factionPoints_[1]);
    res->getInt("faction_points3", info->factionPoints_[2]);
    res->getInt("clanid", info->guildId_);
    res->getInt("clan_contribution", info->guildContribution_);
    res->getInt("clan_rank", info->guildRank_);
    res->getInt("pk_flag", info->pkFlag_);
    res->getInt("stamina", info->stamina_);

    // TODO : write the pat initialization code
    auto skills = entity.assign<Skills>();
    auto sks = database.QStore(fmt::format("SELECT id, level from skill where char_id = {};", charId));
    if (sks) {
        size_t i = 0;
        for (auto &sk : *sks) {
            if (i >= 120)
                break;
            sk->getInt("id", skills->skills_[i].id_);
            sk->getInt("level", skills->skills_[i].level_);
            ++i;
        }
    }

    // TODO : write the hotbar table and loading code
    entity.assign<Hotbar>();

    entity.assign<StatusEffects>();
    entity.assign<RidingItems>();
    entity.assign<BulletItems>();

    // TODO : write the inventory code
    auto inventory = entity.assign<Inventory>();
    res = database.QStore(fmt::format("CALL get_inventory({});", charId));
    if (!res) {
        entity.destroy();
        return Entity();
    }
    for (auto &it : *res) {
        size_t slot;
        it->getInt("slot", slot);
        if (slot >= Inventory::maxItems)
            continue; // TODO : add a warning about that slot
        inventory->items_[slot].loadFromDatabase(*it);
    }
    Systems::UpdateSystem::calculateSpeed(entity);

    registerEntity(entity);
    return entity;
}

void EntitySystem::saveCharacter(uint32_t charId, Entity entity) {
    if (!entity)
        return;
    (void)charId;
}