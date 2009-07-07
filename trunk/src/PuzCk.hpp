// unsigned short is used for 2-byte numbers,
// while char is used for 1-byte numbers

struct PuzCk
{
	unsigned short c_primary;         //
	unsigned char  formatstr[12];     // 'ACROSS&DOWN\0'
	unsigned short c_cib;             //
	unsigned char  c_masked[8];       //
	unsigned char  version[4];        // '1.2\0' or something like it
	unsigned short noise1c;           // 
	unsigned short xunk1e;            //  All of this noise and unknown stuff
	unsigned short noise20;           //     can safely be set to 00
	unsigned short noise22;           //  Across Lite defaults to not changing
	unsigned short noise24;           //     anything it doesn't need to, so
	unsigned short noise26;           //     these regions are preserved
	unsigned short noise28;           //
	unsigned short noise2a;           //
	unsigned char  width;             // width, height, and num_clues make up
	unsigned char  height;            //     the section that initializes all
	unsigned short num_clues;         //     the checksums (referred to as "cib")
	unsigned short xunk30;            //
	unsigned short xunk32;            //
};


// Perhaps all the blank space could be used for recording
//    time, score, a flag to tell if the puzzle is complete,
//    or something like that
