require "puz_overrides"

dofile "init.lua"

library("luapuz", "puz")

namespace{"std", register=false}
typedef{"vector<puz::Square*>", luatype="LUA_TTABLE",
        headers={"<vector>", "puz/Square.hpp"},
        get = "luapuz_checkSquareVector",
        push = "luapuz_pushSquareVector",
        check_func=overrides.checkSquareVector,
        push_func=overrides.pushSquareVector,
        subst_var="squares"}

namespace "puz"

typedef{"ClueList", luatype="LUA_TTABLE",
        headers={"puz/Clue.hpp", "<algorithm>"},
        check_func=overrides.checkClueList,
        push_func=overrides.pushClueList}

enum{ "GridDirection", header="puz/Square.hpp",
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

    "ACROSS_LITE_MASK"
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

enum { "FindOptions", header="puz/Grid.hpp",
    "WRAP",
    "NO_WRAP",
    "WHITE_SQUARES",

    "FIND_IN_GRID",
    "FIND_IN_WORD",
}

func{"SwapDirection", returns="GridDirection", arg("GridDirection", "dir")}


class{"Square", header="puz/Square.hpp"}
    func{"GetCol", override=overrides.Square_GetCol}
    func{"GetRow", override=overrides.Square_GetRow}

    func{"IsLast", returns="bool",
         arg("GridDirection", "dir"), arg("FindDirection", "inc", "puz::NEXT")}
    func{"IsFirst", returns="bool",
         arg("GridDirection", "dir"), arg("FindDirection", "inc", "puz::NEXT")}

    func{"IsSolutionBlank", returns="bool"}
    func{"IsWhite", returns="bool"}
    func{"IsBlack", returns="bool"}
    func{"IsBlank", returns="bool"}

    func{"GetText", returns="puz::string_t"}
    func{"GetPlainText", returns="char"}
    func{"GetTextSymbol", returns="int", throws=true}
    func{"SetText", arg("puz::string_t", "text"), throws=true}
    func{"HasTextRebus", returns="bool"}
    func{"HasTextSymbol", returns="bool"}

    func{"GetSolution", returns="puz::string_t"}
    func{"GetPlainSolution", returns="char"}
    func{"GetSolutionSymbol", returns="int", throws=true}
    func{"SetSolution", arg("puz::string_t", "solution"), throws=true}
    func{"SetSolution", arg("puz::string_t", "solution"), arg("char", "plain"), throws=true}
    func{"SetPlainSolution", arg("char", "plain")}
    func{"SetSolutionRebus", arg("puz::string_t", "rebus"), throws=true}
    func{"SetSolutionSymbol", arg("int", "symbol"), throws=true}
    func{"HasSolutionRebus", returns="bool"}
    func{"HasSolutionSymbol", returns="bool"}

    func{"Check", returns="bool", arg("bool", "checkBlank", "false"),
                                  arg("bool", "strictRebus", "false") }

    func{"HasNumber", returns="bool"}
    func{"GetNumber", returns="puz::string_t"}
    func{"SetNumber", arg("puz::string_t", "number")}
    func{"SetNumber", arg("int", "number")}

    func{"HasClue", returns="bool", override=overrides.Square_HasClue}
    func{"SetClue", arg("GridDirection", "dir"), arg("bool", "clue", "true")}

    func{"SetFlag", arg("unsigned short", "flag")}
    func{"AddFlag", arg("unsigned short", "flag"), arg("bool", "doit", "true")}
    func{"RemoveFlag", arg("unsigned short", "flag")}
    func{"ToggleFlag", arg("unsigned short", "flag")}
    func{"GetFlag", returns="unsigned short"}
    func{"HasFlag", arg("unsigned short", "flag"), returns="bool"}

    func{"HasCircle", returns="bool"}
    func{"SetCircle", arg("bool", "doit", "true")}

    func{"IsMissing", returns="bool"}
    func{"SetMissing", arg("bool", "doit", "true")}

    func{"HasColor", returns="bool"}
    func{"SetColor", arg("unsigned char", "red"),
                     arg("unsigned char", "green"),
                     arg("unsigned char", "blue"),}
    func{"RemoveColor"}

    func{"Next", returns="Square *",
         arg("GridDirection", "dir", "puz::ACROSS"),
         arg("FindDirection", "inc", "puz::NEXT")}
    func{"Prev", returns="Square *",
         arg("GridDirection", "dir", "puz::ACROSS"),
         arg("FindDirection", "inc", "puz::NEXT")}

    func{"GetWordStart", returns="Square *", arg("GridDirection", "dir")}
    func{"GetWordEnd", returns="Square *", arg("GridDirection", "dir")}
    func{"HasWord", returns="bool", arg("GridDirection", "dir")}

    func{"IsValidString", static=true, returns="bool", arg("puz::string_t", "str")}
    func{"IsSymbol", static=true, returns="bool", arg("puz::string_t", "str")}


class{"Grid", header="puz/Grid.hpp"}
    func{"_index", override=overrides.Grid__index}
    func{"_newindex", override=overrides.Grid__newindex}

    func{"Grid", arg("size_t", "width", "0"), arg("size_t", "height", "0")}

    func{"NumberGrid"}

    func{"SetSize", arg("size_t", "width"), arg("size_t", "height")}
    func{"GetWidth", returns="size_t"}
    func{"GetHeight", returns="size_t"}
    func{"LastRow", override=overrides.Grid_LastRow}
    func{"LastCol", override=overrides.Grid_LastCol}

    func{"IsEmpty", returns="bool"}
    func{"Clear"}

    func{"First", returns="Square *"}
    func{"Last", returns="Square *"}

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

    func{"CheckGrid", returns="vector<puz::Square*>", override=overrides.Grid_CheckGrid}
    func{"CheckWord", override=overrides.Grid_CheckWord}
    func{"CheckSquare", returns="bool",
                        arg("Square &", "square"),
                        arg("bool", "checkBlank", "false"),
                        arg("bool", "strictRebus", "false")}

    -- Find functions
    func{"FindSquare", override=overrides.Grid_FindSquare,
                       arg("GridDirection", "direction"),
                       arg("FindDirection", "increment")}

class{"Puzzle", header="puz/Puzzle.hpp", cppheader="luapuz_puz_Puzzle_helpers.hpp"}

    func{"Puzzle", override=overrides.Puzzle_Puzzle}
    func{"Load", override=overrides.Puzzle_Load}
    func{"Save", override=overrides.Puzzle_Save}

    func{"CanLoad", static=true, returns="bool", arg("puz::string_t", "filename")}
    func{"CanSave", static=true, returns="bool", arg("puz::string_t", "filename")}

    func{"Clear"}

    -- Misc
    func{"GetVersion", returns="short"}
    func{"IsScrambled", returns="bool"}
    func{"IsOk", returns="bool"}
    func{"SetOk", arg("bool", "ok")}
    func{"TestOk", throws=true}

    -- Data
    property{"puz::string_t", "Author"}
    property{"puz::string_t", "Title"}
    property{"puz::string_t", "Copyright"}
    property{"Grid &", "Grid"}
    property{"int", "Time"}
    func{"IsTimerRunning", returns="bool"}
    func{"SetTimerRunning", arg("bool", "running")}
    property{"puz::string_t", "Notes"}

    func{"GetAcross", returns="ClueList &"}
    func{"GetDown", returns="ClueList &"}
    func{"SetAcross", arg("ClueList &", "cluelist")}
    func{"SetDown", arg("ClueList &", "cluelist")}

    func{"NumberClues", throws=true}
    func{"NumberGrid"}


bind.run()
