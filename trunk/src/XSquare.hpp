// TODO:
//   - Make the chars wxChars, so we can better support unicode
//   - Or perhaps have both standard and unicode m_solution

#ifndef X_SQUARE_H
#define X_SQUARE_H


// GEXT flags (just learned the one for X and for a circle!)
// Perhaps there are some flags that use 0x01 - 0x08?
const unsigned char XFLAG_CLEAR  = 0x00;
const unsigned char XFLAG_BLACK  = 0x10;
const unsigned char XFLAG_X      = 0x20;
const unsigned char XFLAG_RED    = 0x40;
const unsigned char XFLAG_CIRCLE = 0x80;


// Clue types
enum ClueType
{
	NO_CLUE     = 0x00,
	ACROSS_CLUE = 0x01,
	DOWN_CLUE   = 0x02,
};

// Forward/backward
enum FindIncrement
{
	FIND_PREV,
	FIND_NEXT,
};


// Directions
enum GridDirection
{
	DIR_ACROSS,
	DIR_DOWN
};

// Macro to simplify GetX() functions
#define GET_FUNCTION(type, declaration, member)         \
	const type declaration const { return member; }   \
          type declaration       { return member; }   \



class XGrid;

class XSquare
{
friend class XGrid;

public:
	// Defaults to a black square
	explicit XSquare(int a_col = -1,
	                 int a_row = -1,
	                 char a_solution = '.',
	                 char a_text = '.',
					 unsigned char a_flag = XFLAG_CLEAR,
					 short a_number = 0,
					 short a_clueFlag = NO_CLUE)
		: col(a_col),
		  row(a_row),
		  solution(a_solution),
		  text(a_text),
		  flag(a_flag),
		  number(a_number),
		  clueFlag(a_clueFlag)
	{
		clue[DIR_ACROSS] = 0;
		clue[DIR_DOWN]   = 0;

		m_next[DIR_ACROSS][FIND_PREV] = NULL;
		m_next[DIR_ACROSS][FIND_NEXT] = NULL;
		m_next[DIR_DOWN]  [FIND_PREV] = NULL;
		m_next[DIR_DOWN]  [FIND_NEXT] = NULL;

		m_isLast[DIR_ACROSS][FIND_PREV] = false;
		m_isLast[DIR_ACROSS][FIND_NEXT] = false;
		m_isLast[DIR_DOWN]  [FIND_PREV] = false;
		m_isLast[DIR_DOWN]  [FIND_NEXT] = false;

		m_wordStart[DIR_ACROSS] = NULL;
		m_wordEnd  [DIR_ACROSS] = NULL;
		m_wordStart[DIR_DOWN]   = NULL;
		m_wordEnd  [DIR_DOWN]   = NULL;
	}

	~XSquare() {}

	// Location information
	int col;
	int row;

	// square information
	char  solution;
	char  text;
	unsigned char flag;

	// Clue information
	short number;
	short clueFlag;
	short clue[2]; // clue this belongs to; across and down

	// Flag functions
	void ReplaceFlag (unsigned char flag1, unsigned char flag2)
		{ RemoveFlag(flag1); AddFlag(flag2); }
	void AddFlag     (unsigned char a_flag)       { flag |=   a_flag; }
	void RemoveFlag  (unsigned char a_flag)       { flag &= ~ a_flag; }
	bool HasFlag     (unsigned char a_flag) const
		{ return (flag & a_flag) != 0; }

	// Information functions
	bool IsWhite() const { return ! IsBlack(); }
	bool IsBlack() const { return solution == '.'; }
	bool IsBlank() const { return text == '-'; }

	bool Check(bool checkBlank = false)  const
	{
		if (IsBlack() || (IsBlank() && ! checkBlank))
			return true;

		return solution == text;
	}

	// Linked-list
	//     - functions (rather than member access) needed for const-corectness
	GET_FUNCTION(XSquare *,
		        Next(bool direction = DIR_ACROSS, bool increment = FIND_NEXT),
				m_next[direction][increment])

	GET_FUNCTION(XSquare *,
		        Prev(bool direction = DIR_ACROSS, bool increment = FIND_NEXT),
				m_next[direction][1-increment])

	GET_FUNCTION(XSquare *,
		        WordStart(bool direction),
				m_wordStart[direction])

	GET_FUNCTION(XSquare *,
		        WordEnd(bool direction),
				m_wordEnd[direction])


	// Test to see if word is the last or first in its column/row
	bool IsLast(bool direction, int increment = FIND_NEXT) const
	{
		return m_isLast [direction][increment];
	}

	bool IsFirst(bool direction, int increment = FIND_NEXT) const
	{
		return IsLast(direction, ! increment);
	}

protected:
	// Note that these protected members will be set by XGrid
	// when the grid is resized

	// Linked-list functionality (across/down, previous/next)
	// This is incredibly important, as it makes searching the grid ~20x faster,
	//     because there are no calls to At() and no loop variables
	XSquare * m_next[2][2];

	// last or first in column row (based on direction and increment)
	bool m_isLast[2][2];

	// These two aren't separated by increment (start/end) because they will 
	// only be called when it is known whether start or end is required
	XSquare * m_wordStart[2];
	XSquare * m_wordEnd  [2];
};


#undef GET_FUNCTION

#endif // X_SQUARE_H
