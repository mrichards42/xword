require "puz_overrides"

dofile "init.lua"

library("luapuz", "puz")

namespace "puz"

enum{ "GridDirection", header="puz/Square.hpp",
    "NONE",
    "ACROSS",
    "DOWN"
}

enum{ "FindDirection", header="puz/Square.hpp",
    "PREV",
    "NEXT"
}

enum{ "GextFlag", header="puz/Square.hpp",
    "FLAG_CLEAR",
    "FLAG_PENCIL",
    "FLAG_BLACK",
    "FLAG_X",
    "FLAG_RED",
    "FLAG_CIRCLE",
}

enum { "CheckTest", header="puz/Grid.hpp",
    "NO_CHECK_BLANK",
    "CHECK_BLANK",
}

enum { "GridFlag", header="puz/Grid.hpp",
    "FLAG_NORMAL",
    "FLAG_NO_SOLUTION",
    "FLAG_SCRAMBLED",
}

enum { "GridType", header="puz/Grid.hpp",
    "TYPE_NORMAL",
    "TYPE_DIAGRAMLESS",
}

func{"SwapDirection", returns="GridDirection", arg("GridDirection", "dir")}


class{"Square", header="puz/Square.hpp"}
    func{"GetCol", override=overrides.Square_GetCol}
    func{"GetRow", override=overrides.Square_GetRow}

    func{"IsLast", returns="bool",
         arg("GridDirection", "dir"), arg("FindDirection", "inc", "puz::NEXT")}
    func{"IsFirst", returns="bool",
         arg("GridDirection", "dir"), arg("FindDirection", "inc", "puz::NEXT")}

    func{"IsWhite", returns="bool"}
    func{"IsBlack", returns="bool"}
    func{"IsBlank", returns="bool"}

    func{"GetText", returns="std::string"}
    func{"GetPlainText", returns="char"}
    func{"GetTextSymbol", returns="unsigned short", throws=true}
    func{"SetText", arg("std::string", "text"), throws=true}
    func{"HasTextRebus", returns="bool"}
    func{"HasTextSymbol", returns="bool"}

    func{"GetSolution", returns="std::string"}
    func{"GetPlainSolution", returns="char"}
    func{"GetSolutionSymbol", returns="unsigned short", throws=true}
    func{"SetSolution", arg("std::string", "solution"), throws=true}
    func{"SetSolution", arg("std::string", "solution"), arg("char", "plain"), throws=true}
    func{"SetPlainSolution", arg("char", "plain")}
    func{"SetSolutionRebus", arg("std::string", "rebus"), throws=true}
    func{"SetSolutionSymbol", arg("unsigned short", "symbol"), throws=true}
    func{"HasSolutionRebus", returns="bool"}
    func{"HasSolutionSymbol", returns="bool"}

    func{"Check", returns="bool", arg("bool", "checkBlank")}

    func{"GetNumber", returns="short"}
    func{"HasClue", returns="bool"}
    func{"HasClue", returns="bool", arg("GridDirection", "dir")}

    func{"SetFlag", arg("unsigned char", "flag")}
    func{"AddFlag", arg("unsigned char", "flag")}
    func{"RemoveFlag", arg("unsigned char", "flag")}
    func{"ToggleFlag", arg("unsigned char", "flag")}
    func{"GetFlag", returns="unsigned char"}
    func{"HasFlag", arg("unsigned char", "flag"), returns="bool"}
    func{"ReplaceFlag",
         arg("unsigned char", "flag1"), arg("unsigned char", "flag2")}

    func{"Next", returns="Square *",
         arg("GridDirection", "dir", "puz::ACROSS"),
         arg("FindDirection", "inc", "puz::NEXT")}
    func{"Prev", returns="Square *",
         arg("GridDirection", "dir", "puz::ACROSS"),
         arg("FindDirection", "inc", "puz::NEXT")}

    func{"GetWordStart", returns="Square *", arg("GridDirection", "dir")}
    func{"GetWordEnd", returns="Square *", arg("GridDirection", "dir")}

    func{"IsValidString", static=true, returns="bool", arg("std::string", "str")}
    func{"IsValidChar", static=true, returns="bool", arg("char", "ch")}
    func{"IsSymbol", static=true, returns="bool", arg("std::string", "str")}

class{"Grid", header="puz/Grid.hpp"}
    enum {"GridState",
        "GRID_NONE",
        "GRID_SIZE",
        "GRID_ITERATION",
        "GRID_SOLUTION",
        "GRID_ALL",
    }

    func{"_index", override=overrides.Grid__index}
    func{"_newindex", override=overrides.Grid__newindex}

    func{"Grid", arg("size_t", "width", "0"), arg("size_t", "height", "0")}

    func{"SetupGrid", returns="bool", arg("GridState", "state", "puz::Grid::GRID_ALL")}
    func{"IsGridSetup", returns="bool", arg("GridState", "state", "puz::Grid::GRID_ALL")}

    func{"SetSize", arg("size_t", "width"), arg("size_t", "height")}
    func{"GetWidth", returns="size_t"}
    func{"GetHeight", returns="size_t"}
    func{"LastRow", returns="int"}
    func{"LastCol", returns="int"}

    func{"IsEmpty", returns="bool"}
    func{"Clear"}

    func{"First", returns="Square *"}
    func{"Last", returns="Square *"}
    func{"FirstWhite", returns="Square *"}
    func{"LastWhite", returns="Square *"}

    func{"IsScrambled", returns="bool"}
    func{"HasSolution", returns="bool"}
    property{"unsigned short", "Flag"}

    func{"IsDiagramless", returns="bool"}
    property{"unsigned short", "Type"}

    func{"ScrambleSolution", returns="bool", arg("unsigned short", "key", "0")}
    func{"UnscrambleSolution", returns="bool", arg("unsigned short", "key")}
    func{"CheckScrambledGrid", returns="bool"}

    property{"unsigned short", "Key"}
    property{"unsigned short", "Cksum"}

    func{"IsBetween", returns="bool",
                      arg("Square *", "square"),
                      arg("Square *", "start"),
                      arg("Square *", "end")}

    func{"CheckSquare", returns="bool", arg("Square &", "square"), arg("bool", "checkBlank", "false")}

class{"Puzzle", header="puz/Puzzle.hpp", cppheader="luapuz_puz_Puzzle_helpers.hpp"}
    typedef{"ClueList", luatype="LUA_TTABLE",
            check=overrides.checkClueList,
            header="<algorithm>",
            push=overrides.pushClueList}

    func{"Puzzle", override=overrides.Puzzle_Puzzle}
    func{"Load", override=overrides.Puzzle_Load}
    func{"Save", override=overrides.Puzzle_Save}

    func{"AddLoadHandler", override=overrides.Puzzle_AddLoadHandler, static=true}
    func{"AddSaveHandler", override=overrides.Puzzle_AddSaveHandler, static=true}
    func{"CanLoad", static=true, returns="bool", arg("std::string", "filename")}
    func{"CanSave", static=true, returns="bool", arg("std::string", "filename")}

    -- Misc
    func{"GetVersion", returns="short"}
    func{"IsScrambled", returns="bool"}
    func{"IsOk", returns="bool"}
    func{"SetOk", arg("bool", "ok")}
    func{"IsTimerRunning", returns="bool"}
    func{"SetTimerRunning", arg("bool", "running")}

    -- Data
    property{"std::string", "Author"}
    property{"std::string", "Title"}
    property{"std::string", "Copyright"}
    property{"int", "Time"}
    property{"std::string", "Notes"}
    property{"Grid &", "Grid"}

    func{"GetAcross", returns="ClueList &"}
    func{"GetDown", returns="ClueList &"}
    func{"SetAcross", arg("ClueList &", "cluelist")}
    func{"SetDown", arg("ClueList &", "cluelist")}

    func{"RenumberClues"}

bind.run()
