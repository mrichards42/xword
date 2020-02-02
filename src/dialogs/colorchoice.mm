// This file is part of XWord    
// Copyright (C) 2012 Mike Richards ( mrichards42@gmx.com )
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

#include "colorchoice.hpp"
#include <wx/osx/private.h>

void ColorChoice::OSXUpdateAttributedStrings()
{
    NSArray *itemArray = [((NSPopUpButton *) GetPeer()->GetWXWidget()) itemArray];
    // The standard ctrl font size
    NSDictionary *attrs = [NSDictionary dictionaryWithObject:
                                [NSFont systemFontOfSize: 0.0]
                            forKey:NSFontAttributeName];
    for (int i = 1; i < [itemArray count]; i++) {
        NSMenuItem *item = [itemArray objectAtIndex:i];
        // Make a few spaces, and set the background color
        NSMutableAttributedString * as = [[NSMutableAttributedString alloc]
                                          initWithString:@"      "];
        if (s_colors[i].color.IsOk()) {
            [as addAttribute:NSBackgroundColorAttributeName
                value:s_colors[i].color.OSXGetNSColor()
                range:NSMakeRange(0,5)];
        }
        // Add the text description with the standard font size
        [as appendAttributedString:[[NSMutableAttributedString alloc] initWithString:[item title] attributes:attrs]];
        // Set the new label
        [item setAttributedTitle:as];
    }
}
