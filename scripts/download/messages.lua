if not download then download = {} end

download.CLEAR = 9
download.PREPEND = 10
download.APPEND = 11
download.START = 100
download.END = 101
download.STATS = 102
download.UPDATE_STATUS = 103

-- stats enum
download.MISSING = 1
download.EXISTS = 2
download.SOLVING = 3
download.COMPLETE = 4