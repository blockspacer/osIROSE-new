#pragma once

#include "system.h"
#include "hash.h"

#include <unordered_map>
#include <vector>
#include <tuple>

namespace RoseCommon {
class CliAttack;
class CliHpReq;
class CliReviveReq;
}  // namespace RoseCommon

namespace Systems {

class CombatSystem : public System {
public:
  CombatSystem(SystemManager &manager);
  virtual ~CombatSystem() = default;

  virtual void update(EntityManager &es, std::chrono::milliseconds dt) override;

  void apply_damage(Entity defender, Entity attacker, int32_t damage = 0);
  void apply_damage(Entity defender, int32_t damage = 0);
  
  void processAttack(CMapClient &client, Entity entity, const RoseCommon::CliAttack &packet);
  void processHpRequest(CMapClient &client, Entity entity, const RoseCommon::CliHpReq &packet);
  void processReviveRequest(CMapClient &client, Entity entity, const RoseCommon::CliReviveReq &packet);
  
protected:
  Entity get_closest_spawn(Entity player);
  Entity get_saved_spawn(Entity player);
  Entity get_start_spawn();
  void updateHP(Entity e, std::chrono::milliseconds dt);
};

}  // namespace Systems
