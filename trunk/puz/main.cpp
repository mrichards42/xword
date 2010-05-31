#include <fstream>
#include <iostream>
#include "LoadPuz.hpp"
#include "Puzzle.hpp"

void print_puz(puz::Puzzle & puz)
{
    std::cout << "Title:     " << puz.m_title << std::endl;
    std::cout << "Author:    " << puz.m_author << std::endl;
    std::cout << "Copyright: " << puz.m_copyright << std::endl;
    std::cout << "Timer:     " << puz.m_time << " seconds  " << (puz.m_isTimerRunning ? "(running)" : "(paused)") << std::endl;
    std::cout << "Notes:     " << puz.m_notes << std::endl;

    std::cout << "Across:" << std::endl;
    puz::Puzzle::ClueList::const_iterator across_it;
    for (across_it = puz.m_across.begin(); across_it != puz.m_across.end(); ++across_it)
    {
        std::cout << "    " << across_it->Number() << ". " << across_it->Text() << std::endl;
    }

    std::cout << "Down:" << std::endl;
    puz::Puzzle::ClueList::const_iterator down_it;
    for (down_it = puz.m_down.begin(); down_it != puz.m_down.end(); ++down_it)
    {
        std::cout << "    " << down_it->Number() << ". " << down_it->Text() << std::endl;
    }

    std::cout << "User Grid:" << std::endl;
    std::cout << "    ";
    for (puz::Square * square = puz.m_grid.First();
         square != NULL;
         square = square->Next())
    {
        std::cout << square->GetPlainText();
        if (square->IsLast(puz::ACROSS))
        {
            std::cout << std::endl;
            if (! square->IsLast(puz::DOWN))
                std::cout << "    ";
        }
    }

    std::cout << "Solution Grid:" << std::endl;
    std::cout << "    ";
    for (puz::Square * square = puz.m_grid.First();
         square != NULL;
         square = square->Next())
    {
        std::cout << square->GetPlainSolution();
        if (square->IsLast(puz::ACROSS))
        {
            std::cout << std::endl;
            if (! square->IsLast(puz::DOWN))
                std::cout << "    ";
        }
    }
}


#define CheckThrows(name, exception_class, code)               \
try                                                            \
{                                                              \
    code;                                                      \
    std::cout << "*** " << name << " failed." << std::endl;    \
}                                                              \
catch(exception_class &)                                       \
{                                                              \
    std::cout << name << " passed." << std::endl;              \
}


#define CheckNoThrow(name, code)                               \
try                                                            \
{                                                              \
    code;                                                      \
    std::cout << name << " passed." << std::endl;              \
}                                                              \
catch(...)                                                     \
{                                                              \
    std::cout << "*** " << name << " failed." << std::endl;    \
}


#define CheckTrue(name, condition)                               \
{                                                                \
    const bool __DSLKFJ_passed = condition;                      \
    if (__DSLKFJ_passed)                                         \
        std::cout << name << " passed." << std::endl;            \
    else                                                         \
        std::cout << "*** " << name << " failed." << std::endl;  \
}



int main()
{
    // Tests

    CheckTrue("Load normal",
        puz::Puzzle("D:/C++/XWord/test_files/normal.puz").IsOk()
    )
    puz::Puzzle("D:/C++/XWord/test_files/normal.puz").Save("D:/C++/XWord/test_files/normal_save.puz");
    CheckTrue("Load saved",
        puz::Puzzle("D:/C++/XWord/test_files/normal_save.puz").IsOk()
    )

    CheckThrows("Bad Checksum", puz::ChecksumError,
        puz::Puzzle("D:/C++/XWord/test_files/bad_checksum.puz")
    )

    CheckThrows("Bad GEXT", puz::SectionError,
        puz::Puzzle("D:/C++/XWord/test_files/bad_gext.puz")
    )

    CheckThrows("EOF", puz::FatalFileError,
        puz::Puzzle("D:/C++/XWord/test_files/eof.puz")
    )

    CheckThrows("EOF GEXT", puz::SectionError,
        puz::Puzzle("D:/C++/XWord/test_files/eof_gext.puz")
    )

    std::cin.get();
    return 0;
}
