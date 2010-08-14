// This file is part of XWord
// Copyright (C) 2010 Mike Richards ( mrichards42@gmx.com )
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "Puzzle.hpp"
#include "Clue.hpp"
#include "puzstring.hpp"
#include "util.hpp"
#include <expat.h>
#include <fstream>
#include <map>
#include <vector>
#include <stack>

namespace puz {

static const int BUFF_SIZE = 1024;

struct ParserData;
struct Node;

struct Node
{
    typedef void(*Handler)(ParserData *);

    Node * parent;
    std::map<std::string, Node *> children;
    Handler start;
    Handler end;
    std::string name;

    // Constructor
    Node()
        : parent(NULL),
          start(NULL),
          end(NULL)
    {}
    // Destructor
    ~Node()
    {
        // Delete all child nodes.
        std::map<std::string, Node *>::iterator it;
        for (it = children.begin(); it != children.end(); ++it)
        {
            delete it->second;
        }
    }

    Node & AddChild(const std::string & name, Handler start = NULL, Handler end = NULL)
    {
        // Add this to the map
        Node * n = new Node();
        children[name] = n;
        // Fill in data
        n->name = name;
        n->parent = this;
        n->start = start;
        n->end = end;
        return *n;
    }

    Node & AddChildText(const std::string & name, Handler text)
    {
        return AddChild(name, NULL, text);
    }

    bool HasChildren() const { return ! children.empty(); }
};

// User data
struct ParserData
{
    // Constructor
    ParserData(XML_Parser _parser, Puzzle * _puz, Node * _node)
        : parser(_parser),
          puz(_puz),
          node(_node),
          width(-1),
          height(-1),
          clueAlgorithm(false)
    {}

    // Expat callback data
    //--------------------
    XML_Parser parser;

    // Current node
    Node * node;

    // Element stuff
    std::map<string_t, string_t> attrs; // Attributes of this element
    std::string chardata;

    // Stack of unknown elements
    std::stack<std::string> unknownElements;

    // Text
    //-----
    // All text is converted to puz::string_t

    // No further processing
    puz::string_t GetText()
    {
        return decode_utf8(chardata);
    }

    // Strip whitespace from both ends
    puz::string_t GetStrippedText()
    {
        return TrimWhitespace(GetText());
    }

    // Puzzle data
    //------------
    Puzzle * puz;

    // Grid
    int width;
    int height;
    string_t grid;

    // Clues
    std::vector<string_t> clues;
    // Do we need to algorithmically figure out numbers?
    bool clueAlgorithm;
};


// Generic callback functions
static void XMLCALL StartElement(void *, const XML_Char *, const XML_Char **);
static void XMLCALL EndElement  (void *, const XML_Char *);
static void XMLCALL CharData    (void *, const XML_Char *, int);


// Element callback functions
static void OnTitle(ParserData *);
static void OnAuthor(ParserData *);
static void OnCopyright(ParserData *);
static void OnType(ParserData *);
static void OnNotepad(ParserData *);

static void OnSizeRows(ParserData *);
static void OnSizeCols(ParserData *);
static void OnSizeEnd(ParserData *);

static void OnGridStart(ParserData *);
static void OnGridRow(ParserData *);
static void OnGridEnd(ParserData *);

static void OnCircle(ParserData *);

static void OnRebus(ParserData *);

static void OnShade(ParserData *);

static void OnClue(ParserData *);
static void OnCluesEnd(ParserData *);


void LoadXPF(Puzzle * puz, const string_t & filename, void * /* dummy */)
{
    XML_Parser p = XML_ParserCreate(NULL);
    if (! p)
        throw FatalFileError();

    // XML element structure.
    Node doc;
    Node & puzzles = doc.AddChild("Puzzles");
        Node & puzzle = puzzles.AddChild("Puzzle");
            puzzle.AddChildText("Title", OnTitle);
            puzzle.AddChildText("Author", OnAuthor);
            puzzle.AddChildText("Copyright", OnCopyright);
            puzzle.AddChildText("Type", OnType);
            puzzle.AddChildText("Notepad", OnNotepad);
            Node & size = puzzle.AddChild("Size", NULL, OnSizeEnd);
                size.AddChildText("Rows", OnSizeRows);
                size.AddChildText("Cols", OnSizeCols);
            Node & grid = puzzle.AddChild("Grid", OnGridStart, OnGridEnd);
                grid.AddChildText("Row", OnGridRow);
            Node & circles = puzzle.AddChild("Circles");
                circles.AddChildText("Circle", OnCircle);
            Node & rebus = puzzle.AddChild("RebusEntries");
                rebus.AddChildText("Rebus", OnRebus);
            Node & shades = puzzle.AddChild("Shades");
                shades.AddChildText("Shade", OnShade);
            Node & clues = puzzle.AddChild("Clues", NULL, OnCluesEnd);
                clues.AddChildText("Clue", OnClue);

    // User data
    ParserData data(p, puz, &doc);

    XML_SetUserData(p, &data);
    XML_SetElementHandler(p, StartElement, EndElement); 
    XML_SetCharacterDataHandler(p, CharData);

    try
    {
        // Read the file
        std::ifstream stream(filename.c_str());
        for (;;)
        {
            void * buff = XML_GetBuffer(p, BUFF_SIZE);
            if (buff == NULL)
                throw FatalFileError();
            stream.read(static_cast<char*>(buff), BUFF_SIZE);
            const int bytes_read = stream.gcount();
            if (! XML_ParseBuffer(p, bytes_read, bytes_read == 0))
            {
                throw FatalFileError(std::string("Error loading XML file: ") +
                                     XML_ErrorString(XML_GetErrorCode(p)) + 
                                     " @ line: " + encode_utf8(ToString(XML_GetCurrentLineNumber(p))) +
                                     ", col: " + encode_utf8(ToString(XML_GetCurrentColumnNumber(p))));
            }
            if (bytes_read == 0)
                break;
        }
    }
    catch (...)
    {
        XML_ParserFree(p);
        puz->SetOk(false);
        throw;
    }
    XML_ParserFree(p);
    puz->SetOk(true);
}


// ------------------------------------------------------------------
// Generic callbacks
// ------------------------------------------------------------------
void XMLCALL StartElement(void * _data,
                          const XML_Char * name,
                          const XML_Char **attr)
{
    ParserData * data = (ParserData*)_data;
    data->chardata.clear();

    Node * node = data->node;

    // Do we have a node?
    if (! node)
        throw FatalFileError("Unknown element");

    // Should this node have children?
    if (! node->HasChildren())
        throw FileError(std::string("Unexpected child node ") +
                            "\"" + name + "\" of node " +
                            "\"" + node->name + "\".");

    // Make the attributes table
    data->attrs.clear();
    // attr = { "name", "value", "name", "value", NULL }
    for (; *attr; ++attr)
    {
        const string_t & name = decode_utf8(*attr);
        ++attr;
        const string_t & value = decode_utf8(*attr);
        data->attrs[name] = value;
    }

    // Look for a handler for this element
    std::map<std::string, Node *>::iterator it;
    it = node->children.find(name);
    if (it != node->children.end())
    {
        // Call the start handler if any
        if (node->start)
            node->start(data);
        // Set the new node
        data->node = it->second;
    }
    else
    {
        // Add this element to the stack of unknown elements.
        data->unknownElements.push(name);
    }
}

void XMLCALL EndElement(void * _data,
                        const XML_Char * name)
{
    ParserData * data = (ParserData*)_data;

    // Check to see if we are nested under an unknown element.
    if (! data->unknownElements.empty())
    {
        data->unknownElements.pop();
        return;
    }

    Node * node = data->node;

    // Call the end handler if any
    if (node->end)
        node->end(data);
    // Revert to the parent node.
    data->node = node->parent;
}

void XMLCALL CharData(void * _data,
                      const XML_Char * s,
                      int len)
{
    ParserData * data = (ParserData*)_data;
    data->chardata.append(s, len);
}



// ------------------------------------------------------------------
// Element callbacks
// ------------------------------------------------------------------
// Author
void OnAuthor(ParserData * data)
{
    data->puz->SetAuthor(data->GetStrippedText());
}

// Title
void OnTitle(ParserData * data)
{
    data->puz->SetTitle(data->GetStrippedText());
}


// Copyright
void OnCopyright(ParserData * data)
{
    // Add the copyright symbol
    data->puz->SetCopyright(decode_puz("\xa9 ") + data->GetStrippedText());
}


// Type
void OnType(ParserData * data)
{
    if (data->GetStrippedText() == puzT("diagramless"))
        data->puz->GetGrid().SetType(TYPE_DIAGRAMLESS);
}


// Notepad
void OnNotepad(ParserData * data)
{
    data->puz->SetNotes(data->GetText());
}


// Size
void OnSizeRows(ParserData * data)
{
    data->height = ToInt(data->GetStrippedText());
}

void OnSizeCols(ParserData * data)
{
    data->width = ToInt(data->GetStrippedText());
}

void OnSizeEnd(ParserData * data)
{
    if (data->width < 1 || data->height < 1)
        throw InvalidGrid();
    data->puz->GetGrid().SetSize(data->width, data->height);
}



// Grid
void OnGridStart(ParserData * data)
{
    if (data->width == -1 || data->height == -1)
        throw FatalFileError("Grid size not specified");
    data->grid.reserve(data->width * data->height);
}

void OnGridRow(ParserData * data)
{
    data->grid.append(data->GetText());
}

void OnGridEnd(ParserData * data)
{
    string_t::iterator it = data->grid.begin();
    string_t::iterator end = data->grid.end();
    for (Square * square = data->puz->GetGrid().First();
         square != NULL;
         square = square->Next())
    {
        if (it == end)
            throw FatalFileError("Not enough squares in the grid.");
        if (*it == puzT('.'))
            square->SetSolution(Square::Black);
        else if (*it == puzT(' '))
            square->SetSolution(Square::Blank);
        else if (*it == puzT('~'))
            square->SetMissing();
        else
            square->SetSolution(string_t(1, *it));
        ++it;
    }
    if (it != end)
        throw FatalFileError("Too many squares in the grid.");
}



// Circles
void OnCircle(ParserData * data)
{
    int row = ToInt(data->attrs[puzT("Row")]) - 1;
    int col = ToInt(data->attrs[puzT("Col")]) - 1;

    Grid & grid = data->puz->GetGrid();
    if (row < 0 || row > grid.LastRow()
        || col < 0 || col > grid.LastCol())
    {
        throw FileError("Circle square is out of range");
    }
    else
    {
        grid.At(col, row).SetCircle();
    }
}


// Rebus
void OnRebus(ParserData * data)
{
    int row = ToInt(data->attrs[puzT("Row")]) - 1;
    int col = ToInt(data->attrs[puzT("Col")]) - 1;
    const string_t & plain = data->attrs[puzT("Short")];
    const string_t & rebus = data->GetStrippedText();

    Grid & grid = data->puz->GetGrid();
    if (row < 0 || row > grid.LastRow()
        || col < 0 || col > grid.LastCol())
    {
        throw FileError("Rebus square is out of range");
    }
    else if (rebus.empty())
    {
        // Can't set the rebus if it's empty
    }
    else
    {
        Square & square = grid.At(col, row);
        if (plain.empty())
        {
            if (square.IsSolutionBlank())
                square.SetSolutionRebus(rebus);
            else
                square.SetSolution(rebus);
        }
        else
        {
            square.SetSolution(rebus, Square::ToPlain(plain));
        }
    }
}

// Shades
static unsigned char ParseHex(char_t hi, char_t lo)
{
    if (! isxdigit(hi) || ! isxdigit(lo))
        throw FileError("Invalid color string");
    unsigned char b1 = isdigit(hi) ? hi - 48 : toupper(hi) - 65 + 10;
    unsigned char b2 = isdigit(lo) ? lo - 48 : toupper(lo) - 65 + 10;
    return (b1 << 4) + b2;
}

void OnShade(ParserData * data)
{
    int row = ToInt(data->attrs[puzT("Row")]) - 1;
    int col = ToInt(data->attrs[puzT("Col")]) - 1;
    const string_t & color = data->GetStrippedText();

    Grid & grid = data->puz->GetGrid();
    if (row < 0 || row > grid.LastRow()
        || col < 0 || col > grid.LastCol())
    {
        throw FileError("Shade square is out of range");
    }
    else if (color.empty())
    {
        // Can't set a blank color
    }
    else
    {
        Square & square = grid.At(col, row);
        if (color == puzT("gray"))
        {
            square.SetColor(192, 192, 192);
        }
        else
        {
            // Parse the color: #rrggbb or #rgb
            size_t len = color.size();
            if (! (color.at(0) == puzT('#') && (len == 4 || len == 7)))
                throw FileError("Invalid color string");
            if (len == 4)
                square.SetColor(ParseHex(color[1], color[1]),
                                ParseHex(color[2], color[2]),
                                ParseHex(color[3], color[3]));
            else
                square.SetColor(ParseHex(color[1], color[2]),
                                ParseHex(color[3], color[4]),
                                ParseHex(color[5], color[6]));
        }
    }
}


// Clues
void OnClue(ParserData * data)
{
    int row = ToInt(data->attrs[puzT("Row")]) - 1;
    int col = ToInt(data->attrs[puzT("Col")]) - 1;
    const string_t & number = data->attrs[puzT("Num")];
    const string_t & direction = data->attrs[puzT("Dir")];

    Clue clue;
    clue.SetText(data->GetText());

    Grid & grid = data->puz->GetGrid();
    // Did we get a row and col?
    if (data->attrs[puzT("Row")].empty() || row < -1 ||
        data->attrs[puzT("Col")].empty() || col < -1 ||
        direction.empty())
    {
        // If we're going to algorithmically number the
        // grid, no need to do all the rest of this work
        data->clueAlgorithm = true;
        data->clues.push_back(clue.GetText());
        return;
    }

    try
    {
        Square * square = &grid.At(col, row);
        if (! square->IsWhite())
            throw FileError("Clued squares must be white.");
        square->SetNumber(number);
        clue.SetNumber(number);

        if (direction == puzT("Across"))
            square->SetClue(ACROSS, true);
        else if (direction == puzT("Down"))
            square->SetClue(DOWN, true);

        data->puz->GetClues(direction).push_back(clue);
    }
    catch (std::out_of_range &)
    {
        throw FileError("Clue square out of range.");
    }

    // Add it to the master list as well, just in case we have to renumber things.
    data->clues.push_back(clue.GetText());
}

void OnCluesEnd(ParserData * data)
{
    if (data->clueAlgorithm)
        data->puz->SetAllClues(data->clues);
    // else the clues and numbers should have already been set.
}


} // namespace puz

