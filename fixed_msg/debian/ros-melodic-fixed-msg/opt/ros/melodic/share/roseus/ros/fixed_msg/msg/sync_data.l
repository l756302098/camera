;; Auto-generated. Do not edit!


(when (boundp 'fixed_msg::sync_data)
  (if (not (find-package "FIXED_MSG"))
    (make-package "FIXED_MSG"))
  (shadow 'sync_data (find-package "FIXED_MSG")))
(unless (find-package "FIXED_MSG::SYNC_DATA")
  (make-package "FIXED_MSG::SYNC_DATA"))

(in-package "ROS")
;;//! \htmlinclude sync_data.msg.html


(defclass fixed_msg::sync_data
  :super ros::object
  :slots (_stamp _version _device_id _pos_x _pos_y _pos_z _qua_x _qua_y _qua_z _qua_w _horizontal _vertical _temperature _v_format _v_data ))

(defmethod fixed_msg::sync_data
  (:init
   (&key
    ((:stamp __stamp) "")
    ((:version __version) 0)
    ((:device_id __device_id) 0)
    ((:pos_x __pos_x) 0.0)
    ((:pos_y __pos_y) 0.0)
    ((:pos_z __pos_z) 0.0)
    ((:qua_x __qua_x) 0.0)
    ((:qua_y __qua_y) 0.0)
    ((:qua_z __qua_z) 0.0)
    ((:qua_w __qua_w) 0.0)
    ((:horizontal __horizontal) 0)
    ((:vertical __vertical) 0)
    ((:temperature __temperature) (make-array 0 :initial-element 0 :element-type :char))
    ((:v_format __v_format) "")
    ((:v_data __v_data) (make-array 0 :initial-element 0 :element-type :char))
    )
   (send-super :init)
   (setq _stamp (string __stamp))
   (setq _version (round __version))
   (setq _device_id (round __device_id))
   (setq _pos_x (float __pos_x))
   (setq _pos_y (float __pos_y))
   (setq _pos_z (float __pos_z))
   (setq _qua_x (float __qua_x))
   (setq _qua_y (float __qua_y))
   (setq _qua_z (float __qua_z))
   (setq _qua_w (float __qua_w))
   (setq _horizontal (round __horizontal))
   (setq _vertical (round __vertical))
   (setq _temperature __temperature)
   (setq _v_format (string __v_format))
   (setq _v_data __v_data)
   self)
  (:stamp
   (&optional __stamp)
   (if __stamp (setq _stamp __stamp)) _stamp)
  (:version
   (&optional __version)
   (if __version (setq _version __version)) _version)
  (:device_id
   (&optional __device_id)
   (if __device_id (setq _device_id __device_id)) _device_id)
  (:pos_x
   (&optional __pos_x)
   (if __pos_x (setq _pos_x __pos_x)) _pos_x)
  (:pos_y
   (&optional __pos_y)
   (if __pos_y (setq _pos_y __pos_y)) _pos_y)
  (:pos_z
   (&optional __pos_z)
   (if __pos_z (setq _pos_z __pos_z)) _pos_z)
  (:qua_x
   (&optional __qua_x)
   (if __qua_x (setq _qua_x __qua_x)) _qua_x)
  (:qua_y
   (&optional __qua_y)
   (if __qua_y (setq _qua_y __qua_y)) _qua_y)
  (:qua_z
   (&optional __qua_z)
   (if __qua_z (setq _qua_z __qua_z)) _qua_z)
  (:qua_w
   (&optional __qua_w)
   (if __qua_w (setq _qua_w __qua_w)) _qua_w)
  (:horizontal
   (&optional __horizontal)
   (if __horizontal (setq _horizontal __horizontal)) _horizontal)
  (:vertical
   (&optional __vertical)
   (if __vertical (setq _vertical __vertical)) _vertical)
  (:temperature
   (&optional __temperature)
   (if __temperature (setq _temperature __temperature)) _temperature)
  (:v_format
   (&optional __v_format)
   (if __v_format (setq _v_format __v_format)) _v_format)
  (:v_data
   (&optional __v_data)
   (if __v_data (setq _v_data __v_data)) _v_data)
  (:serialization-length
   ()
   (+
    ;; string _stamp
    4 (length _stamp)
    ;; uint8 _version
    1
    ;; uint8 _device_id
    1
    ;; float32 _pos_x
    4
    ;; float32 _pos_y
    4
    ;; float32 _pos_z
    4
    ;; float32 _qua_x
    4
    ;; float32 _qua_y
    4
    ;; float32 _qua_z
    4
    ;; float32 _qua_w
    4
    ;; int32 _horizontal
    4
    ;; int32 _vertical
    4
    ;; uint8[] _temperature
    (* 1    (length _temperature)) 4
    ;; string _v_format
    4 (length _v_format)
    ;; uint8[] _v_data
    (* 1    (length _v_data)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; string _stamp
       (write-long (length _stamp) s) (princ _stamp s)
     ;; uint8 _version
       (write-byte _version s)
     ;; uint8 _device_id
       (write-byte _device_id s)
     ;; float32 _pos_x
       (sys::poke _pos_x (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _pos_y
       (sys::poke _pos_y (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _pos_z
       (sys::poke _pos_z (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _qua_x
       (sys::poke _qua_x (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _qua_y
       (sys::poke _qua_y (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _qua_z
       (sys::poke _qua_z (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _qua_w
       (sys::poke _qua_w (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; int32 _horizontal
       (write-long _horizontal s)
     ;; int32 _vertical
       (write-long _vertical s)
     ;; uint8[] _temperature
     (write-long (length _temperature) s)
     (princ _temperature s)
     ;; string _v_format
       (write-long (length _v_format) s) (princ _v_format s)
     ;; uint8[] _v_data
     (write-long (length _v_data) s)
     (princ _v_data s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; string _stamp
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _stamp (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; uint8 _version
     (setq _version (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _device_id
     (setq _device_id (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; float32 _pos_x
     (setq _pos_x (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _pos_y
     (setq _pos_y (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _pos_z
     (setq _pos_z (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _qua_x
     (setq _qua_x (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _qua_y
     (setq _qua_y (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _qua_z
     (setq _qua_z (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _qua_w
     (setq _qua_w (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; int32 _horizontal
     (setq _horizontal (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; int32 _vertical
     (setq _vertical (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint8[] _temperature
   (let ((n (sys::peek buf ptr- :integer))) (incf ptr- 4)
     (setq _temperature (make-array n :element-type :char))
     (replace _temperature buf :start2 ptr-) (incf ptr- n))
   ;; string _v_format
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _v_format (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; uint8[] _v_data
   (let ((n (sys::peek buf ptr- :integer))) (incf ptr- 4)
     (setq _v_data (make-array n :element-type :char))
     (replace _v_data buf :start2 ptr-) (incf ptr- n))
   ;;
   self)
  )

(setf (get fixed_msg::sync_data :md5sum-) "7bc7618cef60033e2d78462cb5d33ffd")
(setf (get fixed_msg::sync_data :datatype-) "fixed_msg/sync_data")
(setf (get fixed_msg::sync_data :definition-)
      "string stamp
uint8 version
uint8 device_id
float32 pos_x
float32 pos_y
float32 pos_z
float32 qua_x
float32 qua_y
float32 qua_z
float32 qua_w
int32 horizontal
int32 vertical
uint8[] temperature
string v_format
uint8[] v_data
")



(provide :fixed_msg/sync_data "7bc7618cef60033e2d78462cb5d33ffd")


