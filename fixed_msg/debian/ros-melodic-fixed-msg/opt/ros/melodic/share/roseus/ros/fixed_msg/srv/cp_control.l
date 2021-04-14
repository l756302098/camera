;; Auto-generated. Do not edit!


(when (boundp 'fixed_msg::cp_control)
  (if (not (find-package "FIXED_MSG"))
    (make-package "FIXED_MSG"))
  (shadow 'cp_control (find-package "FIXED_MSG")))
(unless (find-package "FIXED_MSG::CP_CONTROL")
  (make-package "FIXED_MSG::CP_CONTROL"))
(unless (find-package "FIXED_MSG::CP_CONTROLREQUEST")
  (make-package "FIXED_MSG::CP_CONTROLREQUEST"))
(unless (find-package "FIXED_MSG::CP_CONTROLRESPONSE")
  (make-package "FIXED_MSG::CP_CONTROLRESPONSE"))

(in-package "ROS")





(defclass fixed_msg::cp_controlRequest
  :super ros::object
  :slots (_id _action _type _value _allvalue ))

(defmethod fixed_msg::cp_controlRequest
  (:init
   (&key
    ((:id __id) 0)
    ((:action __action) 0)
    ((:type __type) 0)
    ((:value __value) 0)
    ((:allvalue __allvalue) (make-array 0 :initial-element 0 :element-type :integer))
    )
   (send-super :init)
   (setq _id (round __id))
   (setq _action (round __action))
   (setq _type (round __type))
   (setq _value (round __value))
   (setq _allvalue __allvalue)
   self)
  (:id
   (&optional __id)
   (if __id (setq _id __id)) _id)
  (:action
   (&optional __action)
   (if __action (setq _action __action)) _action)
  (:type
   (&optional __type)
   (if __type (setq _type __type)) _type)
  (:value
   (&optional __value)
   (if __value (setq _value __value)) _value)
  (:allvalue
   (&optional __allvalue)
   (if __allvalue (setq _allvalue __allvalue)) _allvalue)
  (:serialization-length
   ()
   (+
    ;; int32 _id
    4
    ;; int32 _action
    4
    ;; int32 _type
    4
    ;; int32 _value
    4
    ;; uint32[] _allvalue
    (* 4    (length _allvalue)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; int32 _id
       (write-long _id s)
     ;; int32 _action
       (write-long _action s)
     ;; int32 _type
       (write-long _type s)
     ;; int32 _value
       (write-long _value s)
     ;; uint32[] _allvalue
     (write-long (length _allvalue) s)
     (dotimes (i (length _allvalue))
       (write-long (elt _allvalue i) s)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; int32 _id
     (setq _id (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; int32 _action
     (setq _action (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; int32 _type
     (setq _type (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; int32 _value
     (setq _value (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint32[] _allvalue
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _allvalue (instantiate integer-vector n))
     (dotimes (i n)
     (setf (elt _allvalue i) (sys::peek buf ptr- :integer)) (incf ptr- 4)
     ))
   ;;
   self)
  )

(defclass fixed_msg::cp_controlResponse
  :super ros::object
  :slots (_result ))

(defmethod fixed_msg::cp_controlResponse
  (:init
   (&key
    ((:result __result) 0)
    )
   (send-super :init)
   (setq _result (round __result))
   self)
  (:result
   (&optional __result)
   (if __result (setq _result __result)) _result)
  (:serialization-length
   ()
   (+
    ;; int32 _result
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; int32 _result
       (write-long _result s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; int32 _result
     (setq _result (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;;
   self)
  )

(defclass fixed_msg::cp_control
  :super ros::object
  :slots ())

(setf (get fixed_msg::cp_control :md5sum-) "be6cabd355b563d374071eb0d114fd3d")
(setf (get fixed_msg::cp_control :datatype-) "fixed_msg/cp_control")
(setf (get fixed_msg::cp_control :request) fixed_msg::cp_controlRequest)
(setf (get fixed_msg::cp_control :response) fixed_msg::cp_controlResponse)

(defmethod fixed_msg::cp_controlRequest
  (:response () (instance fixed_msg::cp_controlResponse :init)))

(setf (get fixed_msg::cp_controlRequest :md5sum-) "be6cabd355b563d374071eb0d114fd3d")
(setf (get fixed_msg::cp_controlRequest :datatype-) "fixed_msg/cp_controlRequest")
(setf (get fixed_msg::cp_controlRequest :definition-)
      "int32 id
int32 action
int32 type
int32 value
uint32[] allvalue
---
int32 result

")

(setf (get fixed_msg::cp_controlResponse :md5sum-) "be6cabd355b563d374071eb0d114fd3d")
(setf (get fixed_msg::cp_controlResponse :datatype-) "fixed_msg/cp_controlResponse")
(setf (get fixed_msg::cp_controlResponse :definition-)
      "int32 id
int32 action
int32 type
int32 value
uint32[] allvalue
---
int32 result

")



(provide :fixed_msg/cp_control "be6cabd355b563d374071eb0d114fd3d")


