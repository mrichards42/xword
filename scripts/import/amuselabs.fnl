(local base64 (require :base64))
(local yajl (require :luayajl))
(local {: decode-rawc} (require :import.amuse-rawc.decode))

(macro icollect-2d [[i j cell array2d] ...]
  `(icollect [,i inner# (ipairs ,array2d)]
     (icollect [,j ,cell (ipairs inner#)]
       ,...)))

;; Note that amuse puzzles are organized like box[x][y], but most other formats
;; (including ipuz) are grid[row][col] (which is grid[y][x]), so we have to
;; transpose each grid.
(fn transpose [grid]
  (let [ret (icollect [_ (ipairs (. grid 1))] [])]
    (each [x col (ipairs grid)]
      (each [y cell (ipairs col)]
        (tset ret y x cell)))
    ret))

(fn word->clue [word]
  {:number word.clueNum
   :clue word.clue.clue
   :cells (when word.nBoxes
            ;; 1-based, not 0-based
            (if word.acrossNotDown
              (fcollect [i 1 word.nBoxes]
                [(+ word.x i) (+ word.y 1)])
              (fcollect [i 1 word.nBoxes]
                [(+ word.x 1) (+ word.y i)])))})

(fn cellInfos->grid [cellInfos]
  "Normalize cellInfos into a 2d grid"
  (let [ret {}]
    (when cellInfos
      (each [_ {: x : y &as info} (ipairs cellInfos)]
        (let [x* (+ x 1)
              y* (+ y 1)]
          (tset ret x* (or (. ret x*) {}))
          (tset ret x* y* info))))
    ret))

(fn amuse->ipuz [json]
  (match json
    {: box : h : w}
    (let [black "#"
          empty 0
          cellInfos (cellInfos->grid json.cellInfos)
          solution-grid (icollect-2d [x y solution box]
                          (let [info (or (?. cellInfos x y) {})]
                            (if
                              ;; if this square has a color it can't be void
                              ;; but it can be black
                              info.isVoid (if info.bgColor black yajl.null)
                              (or (= "\0" solution) (= yajl.null solution)) black
                              :else solution)))]
      {:version "http://ipuz.org/v2"
       :kind ["http://ipuz.org/crossword"]
       :dimensions {:width w :height h}
       :title json.title
       :author json.author
       :publisher json.publisher
       :copyright json.copyright
       :puzzle
       (transpose
        (icollect-2d [x y solution box]
          (let [info (or (?. cellInfos x y) {})
                color-hex (string.match (or info.bgColor "") "#?(%x%x%x%x%x%x")]
            {:cell (if (= black (. solution-grid x y))
                     black
                     (or (?. json.clueNums x y)
                         empty))
             :style {:shapebg (if info.isCircle :circle)
                     :highlight (if (and info.bgColor
                                         (< 0 (length info.bgColor))
                                         (not color-hex))
                                  true) :color color-hex
                     :barred (table.concat [(if info.topWall "T" "")
                                            (if info.bottomWall "B" "")
                                            (if info.leftWall "L" "")
                                            (if info.rightWall "R" "")])}
             :value (if (?. json.preRevealedIdxs x y)
                      solution)})))
       :solution (transpose solution-grid)
       :clues {:Across (icollect [_ word (ipairs json.placedWords)]
                         (when word.acrossNotDown
                           (word->clue word)))
               :Down (icollect [_ word (ipairs json.placedWords)]
                       (when (not word.acrossNotDown)
                         (word->clue word)))}})))

(fn add-checker-highlight! [ipuz]
  (each [y row (ipairs ipuz.puzzle)]
    (each [x cell (ipairs row)]
      (when (and (= (% (+ x y) 2) 0)
                 (not cell.style.color))
        (set cell.style.color "eeeeee"))))
  ipuz)

(fn import.amuselabsJSON [p data]
  (let [json (yajl.to_value data)
        ipuz (amuse->ipuz json)]
    (when (= json.set "tny-new-cryptic")
      (add-checker-highlight! ipuz))
    (p:LoadIpuzString (yajl.to_string ipuz))))

(fn import.amuselabsBase64 [p rawc]
  (import.amuselabsJSON p (decode-rawc rawc)))

(fn import.amuselabsHtml [p html]
  (match (html:match "rawc%s*=%s*'([^']+)'")
    rawc (import.amuselabsBase64 p rawc)))

(import.addHandler import.amuselabsJSON :json "amuselabs JSON")
(import.addHandler import.amuselabsHtml :html "amuselabs HTML")
