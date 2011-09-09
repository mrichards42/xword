require 'luapuz'
p = puz.Puzzle([[D:\c++\xword\test_files\normal.puz]])

getmetatable(p).dump_solution = function(self)
    local square = self:GetGrid():First()
    local s = ""
    while square do
        s = s..square:GetSolution()
        if square:IsLast(puz.ACROSS) then
            print(s)
            s = ""
        end
        square = square:Next(puz.ACROSS)
    end
end

getmetatable(p).dump_text = function(self)
    local g = self:GetGrid()
    for row = 0, g:LastRow() do
        local s = ""
        for col = 0, g:LastCol() do
            s = s..g:At(col, row):GetText()
        end
        print(s)
    end
end


p:dump_solution()

--p:dump_grid()
