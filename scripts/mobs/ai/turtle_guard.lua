registerNpc(85, {
  walk_speed        = 180,
  run_speed         = 470,
  scale             = 85,
  r_weapon          = 0,
  l_weapon          = 0,
  level             = 29,
  hp                = 32,
  attack            = 119,
  hit               = 104,
  def               = 116,
  res               = 62,
  avoid             = 23,
  attack_spd        = 90,
  is_magic_damage   = 0,
  ai_type           = 4,
  give_exp          = 36,
  drop_type         = 139,
  drop_money        = 23,
  drop_item         = 60,
  union_number      = 60,
  need_summon_count = 0,
  sell_tab0         = 0,
  sell_tab1         = 0,
  sell_tab2         = 0,
  sell_tab3         = 0,
  can_target        = 0,
  attack_range      = 220,
  npc_type          = 2,
  hit_material_type = 2,
  face_icon         = 0,
  summon_mob_type   = 0,
  quest_type        = 0,
  height            = 0
});

function OnInit(entity)
  return true
end

function OnCreate(entity)
  return true
end

function OnDelete(entity)
  return true
end

function OnDead(entity)
end

function OnDamaged(entity)
end