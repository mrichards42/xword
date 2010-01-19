-- ============================================================================
-- Download message codes
-- ============================================================================

-- This can be used independently of the entire download package
-- e.g. a download thread can require this file to get the correct message
-- codes.
if not download then download = {} end

-- ===========================================================================
-- Everything declared after this statement is part of the download package
-- ===========================================================================
setfenv(1, download)

-- ----------------------------------------------------------------------------
-- Message codes
-- ----------------------------------------------------------------------------
DL_START = 1
DL_PROGRESS = 2
DL_END = 3
DL_MESSAGE = 4
DL_UNKNOWN = -10

DL_ABORT = -1
DL_ERROR = -2
