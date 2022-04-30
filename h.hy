;; hy2py
;; https://github.com/hylang/hy/blob/stable/docs/language/cli.rst
;; from toolz import mapcat as flatmap
(import toolz [mapcat :as flatmap]
        plum  [dispatch]
        e_helper [MsgType ElemLen])

(= MsgType.INIT.value (get (bytes.fromhex "701234") 0))

(defn first [coll]
  (cut coll 1))

(defn take [coll [n 1]]
  (cut coll n))

(defn rest [coll]
  (let [l (len coll)]
    (cut coll 1 l)))

(defn drop [coll n]
  (let [l (len coll)]
    (cut coll n l)))

(defn constantly [x & rest] (fn [_ & _rest] x))

(defn int->bytes [^int x [n 2]]
  (. x (to-bytes n :byteorder "big")))

(defn gen-init-msg [^int id]
  "id should be int. this function will convert it to bytes"
  (let [id-bytes (int->bytes id)]
    (+ (int->bytes MsgType.INIT.value 1) id-bytes)))

;; https://docs.hylang.org/en/stable/language/api.html#id28
;; with-decorator
#@(dispatch
  (defn print-hex [^bytes x]
    (. x (hex))))

#@(dispatch
  (defn print-hex [^int x [n 4]]
    (. (int->bytes x n) (hex))))

;; https://docs.python.org/3/library/stdtypes.html#bytes.hex
;; (defn gen-resp [^bytes msg]
;;   (match (get msg 0)
;;     MsgType.INIT.value        None ;; from client
;;     MsgType.RTMP_EMERG.value  None ;; from client
;;     MsgType.RTMP_STREAM.value "RTMP_STREAM" ;; from server, send ack
;;     MsgType.HEARTBEAT         None ;; invalid
;;     _                         None))
