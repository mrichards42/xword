(local {:decode base64-decode} (require :base64))
(local {:to_value json-decode} (require :luayajl))
(local bigrams (require :import.amuse-rawc.bigram-data))

;;; == Helpers ================================================================

(fn cycle-ipairs-iter [tbl i]
  (match (. tbl (+ 1 i))
    x (values (+ i 1) x)
    nil (values 1 (. tbl 1))))

(fn cycle-ipairs [tbl]
  (values cycle-ipairs-iter tbl 0))

;; Basic algorithm

(fn decode-with-key [rawc key-str ?single]
  (let [key (icollect [c (key-str:gmatch ".")]
              (+ (tonumber c 16) 2))]
    (var pos 1)
    (table.concat
     (icollect [_ chunk-size (if ?single (ipairs key) (cycle-ipairs key))
                :until (< (length rawc) pos)]
       (let [start-pos pos]
         (set pos (+ pos chunk-size))
         (string.reverse (rawc:sub start-pos (- pos 1))))))))


;;; == Heuristic based key guessing ===========================================

(local leading-chars {}) ; just the first character of each bigram
(each [str frequency (pairs bigrams)]
  (let [char (str:sub 2 2)] ; bigrams are reversed, so character 2 is the first
    (tset leading-chars char (+ frequency (or (. leading-chars char) 0)))))

(fn pending-char-count [position]
  ;; base64 strings can be broken into blocks of 4 characters (24 bits) if the
  ;; start index-1 is not a multiple of 4, that means we have some "pending"
  ;; characters at the end of the string (since we're reversing it)
  (% (- 4 position -1) 4))

(fn find-first-candidate [rawc]
  ;; JSON objects always always start with {" which is "ey" in base64, so we're
  ;; guaranteed to have that be the length of the first chunk (though it still
  ;; needs to be within the first 18 characters)
  (let [first-chunk (rawc:sub 1 18)]
    (match (first-chunk:find "ye")
      (_ pos) {:pending 0
               :match "ey"
               :length pos
               :key (string.format "%x" (- pos 2))}
      ;; Rarely there will be whitespace or a newline after the opening bracket
      _ (match (first-chunk:find "we")
          (_ pos) {:pending 0
                   :match "ew"
                   :length pos
                   :key (string.format "%x" (- pos 2))}
          _ (error "Unable to find start of string!")))))

;; Here's how candidate selection works:
;;
;; Using the following run as an example 'abcdefghijklmnopq'
;;
;; With 3 pending
;; (2) .. <-- no information, both pending
;; (3) ... <-- no information, all pending
;; (4) a... <- just the first character (not a bigram)
;; (5) ab... <-- looking at 'ba' with 'edc' pending
;; (6) .bc... <-- looking at 'cba' with 'fed' pending
;; (7) ..cd... <-- looking at 'dcba' with 'gfe' pending
;; (8) a..de... <-- looking at 'edcb' with 'hgf' pending, and 'a'
;;                  starting a new block -- 'a' as a possibility was already
;;                  checked at length 4.
;; 9-17 are just checking successive bigrams: fe, gh, hg, ih, ji, kj, lk, ml, nm, on, po, qp
;;
;; With 2 pending
;; (2) .. <-- no information, both pending
;; (3) a.. <-- just the first character (not a bigram)
;; (4) ab.. <-- looking at 'ba' with 'dc' pending
;; (5) .bc.. <-- looking at 'cba' with 'ed' pending
;; (6) ..cd.. <-- looking at 'dcba' with 'fe' pending
;; (7) a..de.. <-- looking at 'edcb' with 'gf' pending, and 'a' starting a new
;;                  block (which we saw at length 3)
;; 8-17 are the rest of the bigrams
;;
;; With 1 pending
;; (2) a. <-- first character
;; (3) ab. <-- bigram
;; (4) .bc.
;; (5) ..cd.
;; (6) a..de. <-- bigram + first character
;; 7-17 are the rest
;;
;; 0 pending
;; (2) ab <-- bigram
;; (3) .bc
;; (4) ..cd
;; (5) a..de <-- bigram + first character
;; 6-17 are the rest
;;
;; So the three cases are:
;; - between 2 (the minimum length) and the number of pending characters, all
;;   of those need to be considered candidates without checking
;; - at pending + 1 (assuming that's > 2 in total), check the first character
;; - after that, walk down the string, checking successive bigrams
(fn find-candidates [rawc from]
  (let [pending (pending-char-count from)
        end-offset (- 17 pending)
        candidates []]
    ;; With length <= pending, we have no information about these characters
    ;; since they're the end of a block. We have to consider them candidates
    (fcollect [len 2 pending
               :into candidates]
      {: pending
       :length len
       :key (string.format "%x" (- len 2))})
    ;; With length = pending + 1, we can examine just the first character
    (when (<= 1 pending)
      (match (. leading-chars (rawc:sub from from))
        freq (table.insert
              candidates
              {: pending
               :frequency freq
               :match (rawc:sub from from)
               :length (+ pending 1)
               :key (string.format "%x" (- (+ pending 1) 2))})))
    ;; With length > pending + 1, we have a full 2 characters to examine.
    ;; Length can be max 17 (0xf + 2), so we can examine up to 15 - pending
    (fcollect [i from (+ from (- 15 pending))
               :into candidates]
      (match (. bigrams (rawc:sub i (+ 1 i)))
        x {: pending
           :frequency x
           :match (string.reverse (rawc:sub i (+ 1 i)))
           :length (+ (- i from) pending 2)
           :key (string.format "%x" (+ (- i from) pending))}))
    candidates))

;; Now that we have the basic set of candidates, we can refine this a little
;; further: longer length candidates must have a chain of shorter candidates
;; that are 4 characters shorter (e.g. an 11 must have a 7 and a 3).
(fn guess-next-key [rawc from]
  (if (= 1 from)
    [(find-first-candidate rawc)]
    (let [candidates (find-candidates rawc from)
          pending (pending-char-count from)
          lengths {}
          refined-candidates []]
      (each [_ candidate (ipairs candidates)]
        ;; Min length is 2 and blocks are 4 in length, so the first valid lengths
        ;; are between 2 and 6. After that, select candidates that are +4 (one
        ;; block) from a previously selected candidate.
        (when (or (<= 2 candidate.length 6)
                  (. lengths (- candidate.length 4)))
          (tset lengths candidate.length candidate)
          (table.insert refined-candidates candidate)))
      refined-candidates)))


(fn valid-key-prefix? [rawc key]
  "Does this key work to decode the first chunk of the file? This function
  decodes just the first section of rawc using the key and checks to see if it
  produces valid json."
  (let [deobfuscated* (decode-with-key rawc key 1)
        ;; Chop off the end of the result so it is a multiple of 4
        deobfuscated (deobfuscated*:sub 1 (* 4 (math.floor (/ (length deobfuscated*) 4))))]
    ;; (1) base64 decode -- this has to succeed to be a valid key
    (match (pcall #(base64-decode deobfuscated))
      (false err) false
      (true decoded)
      ;; (2) parse the result as json -- if we hit EOF that means everything
      ;; was valid so far, which is good enough for this function.
      (let [(success? err) (pcall #(json-decode decoded))]
        (if
          success? true
          (err:match "premature EOF") true
          false)))))

(fn guess-key* [rawc chains remaining-steps]
  (let [ret {}]
    (each [pos keys (pairs chains)]
      (each [_ candidate (ipairs (guess-next-key rawc pos))]
        (each [_ key (ipairs keys)]
          (let [new-pos (+ pos candidate.length)
                new-key (.. key candidate.key)]
            (when (not (. ret new-pos))
              (tset ret new-pos []))
            (when (valid-key-prefix? rawc new-key)
              ;; see if this works for the whole string, if so we're done
              (when (pcall #(json-decode (base64-decode (decode-with-key rawc new-key))))
                (let [result {1 [new-key]}]
                  (lua "return result")))
              (table.insert (. ret new-pos) new-key))))))
    (if (< 1 remaining-steps)
      (guess-key* rawc ret (- remaining-steps 1))
      ret)))

;; basic cache
(local recent-keys {:idx 0})
(fn cache-key! [key]
  (set recent-keys.idx (+ 1 (% recent-keys.idx 20)))
  (tset recent-keys recent-keys.idx key))

(fn guess-key [rawc ?max-key-len]
  ;; check the cache first
  (each [_ key (ipairs recent-keys)]
    (match (pcall #(json-decode (base64-decode (decode-with-key rawc key))))
      true key))
  (each [_ keys (pairs (guess-key* rawc {1 [""]} (or ?max-key-len 7)))]
    (each [_ key (ipairs keys)]
      (match (pcall #(json-decode (base64-decode (decode-with-key rawc key))))
        true (do (cache-key! key) (lua "return key"))
        (false err) (when false (print key err))))))

;;; Main export

(fn decode-rawc [rawc]
  (match (rawc:match "^([^.]+)%.(.*)$")
    ;; key is attached to the end
    (payload key) (base64-decode (decode-with-key payload key))
    ;; otherwise we have to guess the key
    _ (match (guess-key rawc)
        key (base64-decode (decode-with-key rawc key))
        _ (error "Unable to decode puzzle"))))

{: decode-rawc
 : guess-key
 ;; snake case for lua
 :decode_rawc decode-rawc
 :guess_key guess-key}
