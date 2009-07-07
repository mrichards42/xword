#ifndef PUZ_LOADER_H
#define PUZ_LOADER_H

#include <wx/string.h>
#include <vector>

// forward declaration
class XPuzzle;

// A namespace to encapsulate loading and saving puzzle files
// This works like a class with two static functions
namespace PuzLoader
{
	bool Load(XPuzzle * puz,
		      const wxString & filename,
			  wxString ext = wxEmptyString);

	bool Save(XPuzzle * puz,
		      const wxString & filename,
			  wxString ext = wxEmptyString);

	bool CanSave(const wxString & ext);
	bool CanLoad(const wxString & ext);

	struct TypeDesc {
		wxString ext;
		wxString description;
	};

	// Spits out a string that can be passed to wxWidgets file dialogs
	std::vector<TypeDesc> GetSaveTypes();
	std::vector<TypeDesc> GetLoadTypes();
};

#endif // PUZ_LOADER_H
