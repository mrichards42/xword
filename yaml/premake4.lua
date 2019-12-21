project "yaml"
    -- --------------------------------------------------------------------
    -- General
    -- --------------------------------------------------------------------
    kind "SharedLib"
    language "C"
    files {
        "src/*.h",
        "src/*.c",
    }

    includedirs { "include" }

    targetname "yaml"

    -- See CMakeLists.txt for version constants.
    defines {
        "YAML_DECLARE_EXPORT",
        "YAML_VERSION_MAJOR=0",
        "YAML_VERSION_MINOR=2",
        "YAML_VERSION_PATCH=2",
        "YAML_VERSION_STRING=0.2.2",
    }
