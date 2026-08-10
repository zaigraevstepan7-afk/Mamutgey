std::string Hooks::get_player_name(void* player_move_c)
{
    if (player_move_c == nullptr) return "";
    void* nick_label = (void*)*(uint64_t*)((uint64_t)player_move_c + Offsets::nickLabel);
    void* name_ptr = Functions::TextMeshGetText(nick_label);
    if (name_ptr == nullptr) return "";
    std::string name = ((Unity::System_String*)name_ptr)->ToString();
    return ClientUtil::CleanString(name);
}

void* Hooks::get_player_transform(void* player)
{
    return (void*)*(uint64_t*)((uint64_t)player + Offsets::myPlayerTransform);
}

bool Hooks::is_player_enemy(void* player)
{
    if (player == nullptr) return false;
    void* nick_label = (void*)*(uint64_t*)((uint64_t)player + Offsets::nickLabel);
    Unity::Color color = {0, 0,  0, 0};
    Functions::TextMeshGetColor(nick_label, &color);
    return color.r == 1 && color.g == 0 && color.b == 0;
}

bool is_my_player_move_c(void* player_move_c)
{
    return Hooks::get_player_name(player_move_c) == "#Player Nickname";
}

bool is_my_player_weapon_sounds(void* weapon_sounds)
{
    void* player_move_c = (void*)*(uint64_t*)((uint64_t)weapon_sounds + Offsets::weaponSoundsPlayerMoveC);
    if (player_move_c == nullptr) return false;
    return is_my_player_move_c(player_move_c);
}

std::string get_player_name_from_weapon_sounds(void* weapon_sounds)
{
    void* player_move_c = (void*)*(uint64_t*)((uint64_t)weapon_sounds + Offsets::weaponSoundsPlayerMoveC);
    if (player_move_c == nullptr) return "";
    return Hooks::get_player_name(player_move_c);
}

Unity::CCamera* find_main_camera()
{
    // Unity::il2cppClass* camera_class = IL2CPP::Class::Find("UnityEngine.Camera");
    // Unity::il2cppObject* camera_type = IL2CPP::Class::GetSystemType(camera_class);
    // Unity::il2cppArray<Unity::CCamera*>* cameras = Unity::Object::FindObjectsOfType<Unity::CCamera>(camera_type);

    // if (cameras == nullptr) return nullptr;
    
    // for (int i = 0; i < cameras->m_uMaxLength; i++)
    // {
    //     Unity::CCamera* camera = cameras->At(i);
    //     std::string name = ClientUtil::CleanString((*camera->GetName()).ToString());
    //     if (name == "ThirdPersonCamera(Clone)") return camera;
    // }
    
    return (Unity::CCamera*)Functions::CameraGetMain();
}

// Hook Functions
inline void(__stdcall* weapon_sounds_original)(void* arg);
inline void __stdcall weapon_sounds_call(void* arg)
{
    if (unlock_weapons_module != nullptr)
    {
        ((ModuleBase*)unlock_weapons_module)->run(arg);
    }
    if (unlock_gadgets_module != nullptr)
    {
        ((ModuleBase*)unlock_gadgets_module)->run(arg);
    }
    if (add_armor_module != nullptr)
    {
        ((ModuleBase*)add_armor_module)->run(arg);
    }
    if (add_pets_module != nullptr)
    {
        ((ModuleBase*)add_pets_module)->run(arg);
    }
    if (add_currency_module != nullptr)
    {
        ((ModuleBase*)add_currency_module)->run(arg);
    }
    
    if (is_my_player_weapon_sounds(arg))
    {
        if (Hooks::our_player != nullptr && aim_bot_module && aim_bot_module->is_using_silent_aim) ((ModuleBase*)aim_bot_module)->run(arg);
        
        for (ModuleBase* weapon_sounds_module : weapon_sounds_modules)
        {
            weapon_sounds_module->run(arg);
        }
    }
    else
    {
        for (ModuleBase* weapon_sounds_other_module : weapon_sound_others_modules)
        {
            weapon_sounds_other_module->run(arg);
        }
    }

    return weapon_sounds_original(arg);
}

inline void(__stdcall* weapon_sounds_late_original)(void* arg);
inline void __stdcall weapon_sounds_late_call(void* arg)
{
    // unused right now
    /*
    if (is_my_player_weapon_sounds(arg))
    {
        
    }
    */

    return weapon_sounds_late_original(arg);
}

std::string random_string( size_t length )
{
    auto randchar = []() -> char
    {
        const char charset[] = { "0123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" };
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}

inline void(__stdcall* player_move_c_original)(void* arg);
inline void __stdcall player_move_c(void* arg)
{
    bool my_player = is_my_player_move_c(arg);
    if (Hooks::fov_changer_module != nullptr)
    {
        if (my_player)
        {
            // Just do this every fucking call innit
            Hooks::main_camera = find_main_camera();
            if (Hooks::main_camera == nullptr) return player_move_c_original(arg);
            Hooks::our_player = arg; // WARN: ALWAYS ALLOW THIS TO BE SET, OTHERWISE BREAKS A LOT OF MODULES
        
            Hooks::fov_changer_module->run(nullptr);
        
            for (ModuleBase* player_move_c_module : player_move_c_modules)
            {
                player_move_c_module->run(arg);
            }
        }
        else
        {
            // Other Players
            if (Hooks::main_camera == nullptr) return player_move_c_original(arg);
            // Functions::TestKicker(arg);
            Hooks::fov_changer_module->run(nullptr);
            esp_module->add_esp(arg);
            working_player_list.push_back(arg);
        
            for (auto player_move_c_others_module : player_move_c_others_modules)
            {
                player_move_c_others_module->run(arg);
            }
        }
    }
    return player_move_c_original(arg);
