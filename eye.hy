#!/usr/bin/env hy

(import
  [time [sleep]]
  [lib.windows [new-renderer new-master new-overview]]
  [lib.osc [nerve-cb! nerve-start]]
  [lib.pyexec [pyexec]])

(require lib.runner)


(runner Eye [self]
        "
        Handles and draws the different visions

        Recives from Coder:
        * `/eye/code    [filename code]`
        * `/eye/delete  [filename]` (not yet implemented)
        * `/eye/move    [oldname newname]`

        Sends to Ear:
        * `/ear/code  [cmd]` signal generated by lib.audio.source

        Recives from Ear:
        * `/eye/audio/[cmd]  [value]` signal handled by lib.audio.source

        Recives from Universe:
        * `[anything]  [?]` signal handled by lib.osc.source (not tested)
        "
        (print "starting eye.hy")

        (setv visions {})

        ; windows
        ; for now just one renderer -> TODO one for each vision
        (setv renderer (new-renderer visions))
        (setv master (new-master renderer))
        (setv overview (new-overview renderer))

        (nerve-cb! "/eye/code"
                   (fn [path args]
                       (setv [name code] args)
                       (if (in name (visions.keys))
                         (.load (get visions name)
                                code)
                         (assoc visions
                                name
                                (new-vision name code)))))

        (setv nerve-stop (nerve-start))

        (import [pyglet])
        (.schedule_interval pyglet.clock
                            (fn [dt]
                                (when self._stop
                                  (.exit pyglet.app) ))
                            (/ 1 120))
        (try
          (.run pyglet.app)
          (catch [KeyboardInterrupt]
                 None))

        (print "\rstopping eye.hy")
        (nerve-stop))


(defn new-vision [path code]
  (setv name
        (get (.split path "/") -1))

  ; stack here the loaded codes, so when everything explodes, we can
  ; always restore the last (opefully) working vision
  (setv stack [])

  (defclass Box []
    "A small sandbox where to run the livecoded part"
    [[draw
       (fn [self])]])

  (setv box (Box))

  (defclass Vision []
    "
    The vision instance
    "
    [[load
       (fn [self code]
           (print "\rloading:" name)
           (setv filename (% "visions/%s" name))
           (try
             (pyexec code box.__dict__)
             (except [e Exception]
                     (print name e))
             (else
               (.append stack code))))]

     [iteration
       (fn [self]
           (try
             (.draw box)
             ; if there is an error and stack is empty you are in the situation
             ; where the FIRST loaded vision is broken, that can be a problem
             ; (for you, if you are livecoding)
             (except [e Exception]
                     (print name name e)
                     (.pop stack)

                     (if stack
                       (pyexec (get stack -1)
                               box.__dict__)
                       (print name "BROKEN!")))))]])

  (setv vision (Vision))
  (.load vision code)
  vision)


(defmain [args]
  (.run (Eye)))
