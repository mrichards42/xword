-- ============================================================================
-- Download message codes
-- ============================================================================

-- This can be used independently of the entire download package
-- e.g. a download thread can require this file to get the correct message
-- codes.
if not download then download = {} end

local P = download

-- ----------------------------------------------------------------------------
-- Message codes
-- ----------------------------------------------------------------------------
P.DL_START = 1
P.DL_PROGRESS = 2
P.DL_END = 3
P.DL_MESSAGE = 4

P.UPDATE_STATS = 5
P.NEW_LAYOUT = 6
P.LAYOUT_START = 7
P.LAYOUT_END = 8

P.DL_UNKNOWN = -10

P.DL_ABORT = -1
P.DL_ERROR = -2
