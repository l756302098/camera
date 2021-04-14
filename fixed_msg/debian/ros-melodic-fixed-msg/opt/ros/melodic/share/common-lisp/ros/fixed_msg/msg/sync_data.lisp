; Auto-generated. Do not edit!


(cl:in-package fixed_msg-msg)


;//! \htmlinclude sync_data.msg.html

(cl:defclass <sync_data> (roslisp-msg-protocol:ros-message)
  ((stamp
    :reader stamp
    :initarg :stamp
    :type cl:string
    :initform "")
   (version
    :reader version
    :initarg :version
    :type cl:fixnum
    :initform 0)
   (device_id
    :reader device_id
    :initarg :device_id
    :type cl:fixnum
    :initform 0)
   (pos_x
    :reader pos_x
    :initarg :pos_x
    :type cl:float
    :initform 0.0)
   (pos_y
    :reader pos_y
    :initarg :pos_y
    :type cl:float
    :initform 0.0)
   (pos_z
    :reader pos_z
    :initarg :pos_z
    :type cl:float
    :initform 0.0)
   (qua_x
    :reader qua_x
    :initarg :qua_x
    :type cl:float
    :initform 0.0)
   (qua_y
    :reader qua_y
    :initarg :qua_y
    :type cl:float
    :initform 0.0)
   (qua_z
    :reader qua_z
    :initarg :qua_z
    :type cl:float
    :initform 0.0)
   (qua_w
    :reader qua_w
    :initarg :qua_w
    :type cl:float
    :initform 0.0)
   (horizontal
    :reader horizontal
    :initarg :horizontal
    :type cl:integer
    :initform 0)
   (vertical
    :reader vertical
    :initarg :vertical
    :type cl:integer
    :initform 0)
   (temperature
    :reader temperature
    :initarg :temperature
    :type (cl:vector cl:fixnum)
   :initform (cl:make-array 0 :element-type 'cl:fixnum :initial-element 0))
   (v_format
    :reader v_format
    :initarg :v_format
    :type cl:string
    :initform "")
   (v_data
    :reader v_data
    :initarg :v_data
    :type (cl:vector cl:fixnum)
   :initform (cl:make-array 0 :element-type 'cl:fixnum :initial-element 0)))
)

(cl:defclass sync_data (<sync_data>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <sync_data>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'sync_data)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name fixed_msg-msg:<sync_data> is deprecated: use fixed_msg-msg:sync_data instead.")))

(cl:ensure-generic-function 'stamp-val :lambda-list '(m))
(cl:defmethod stamp-val ((m <sync_data>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-msg:stamp-val is deprecated.  Use fixed_msg-msg:stamp instead.")
  (stamp m))

(cl:ensure-generic-function 'version-val :lambda-list '(m))
(cl:defmethod version-val ((m <sync_data>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-msg:version-val is deprecated.  Use fixed_msg-msg:version instead.")
  (version m))

(cl:ensure-generic-function 'device_id-val :lambda-list '(m))
(cl:defmethod device_id-val ((m <sync_data>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-msg:device_id-val is deprecated.  Use fixed_msg-msg:device_id instead.")
  (device_id m))

(cl:ensure-generic-function 'pos_x-val :lambda-list '(m))
(cl:defmethod pos_x-val ((m <sync_data>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-msg:pos_x-val is deprecated.  Use fixed_msg-msg:pos_x instead.")
  (pos_x m))

(cl:ensure-generic-function 'pos_y-val :lambda-list '(m))
(cl:defmethod pos_y-val ((m <sync_data>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-msg:pos_y-val is deprecated.  Use fixed_msg-msg:pos_y instead.")
  (pos_y m))

(cl:ensure-generic-function 'pos_z-val :lambda-list '(m))
(cl:defmethod pos_z-val ((m <sync_data>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-msg:pos_z-val is deprecated.  Use fixed_msg-msg:pos_z instead.")
  (pos_z m))

(cl:ensure-generic-function 'qua_x-val :lambda-list '(m))
(cl:defmethod qua_x-val ((m <sync_data>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-msg:qua_x-val is deprecated.  Use fixed_msg-msg:qua_x instead.")
  (qua_x m))

(cl:ensure-generic-function 'qua_y-val :lambda-list '(m))
(cl:defmethod qua_y-val ((m <sync_data>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-msg:qua_y-val is deprecated.  Use fixed_msg-msg:qua_y instead.")
  (qua_y m))

(cl:ensure-generic-function 'qua_z-val :lambda-list '(m))
(cl:defmethod qua_z-val ((m <sync_data>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-msg:qua_z-val is deprecated.  Use fixed_msg-msg:qua_z instead.")
  (qua_z m))

(cl:ensure-generic-function 'qua_w-val :lambda-list '(m))
(cl:defmethod qua_w-val ((m <sync_data>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-msg:qua_w-val is deprecated.  Use fixed_msg-msg:qua_w instead.")
  (qua_w m))

(cl:ensure-generic-function 'horizontal-val :lambda-list '(m))
(cl:defmethod horizontal-val ((m <sync_data>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-msg:horizontal-val is deprecated.  Use fixed_msg-msg:horizontal instead.")
  (horizontal m))

(cl:ensure-generic-function 'vertical-val :lambda-list '(m))
(cl:defmethod vertical-val ((m <sync_data>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-msg:vertical-val is deprecated.  Use fixed_msg-msg:vertical instead.")
  (vertical m))

(cl:ensure-generic-function 'temperature-val :lambda-list '(m))
(cl:defmethod temperature-val ((m <sync_data>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-msg:temperature-val is deprecated.  Use fixed_msg-msg:temperature instead.")
  (temperature m))

(cl:ensure-generic-function 'v_format-val :lambda-list '(m))
(cl:defmethod v_format-val ((m <sync_data>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-msg:v_format-val is deprecated.  Use fixed_msg-msg:v_format instead.")
  (v_format m))

(cl:ensure-generic-function 'v_data-val :lambda-list '(m))
(cl:defmethod v_data-val ((m <sync_data>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-msg:v_data-val is deprecated.  Use fixed_msg-msg:v_data instead.")
  (v_data m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <sync_data>) ostream)
  "Serializes a message object of type '<sync_data>"
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'stamp))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'stamp))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'version)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'device_id)) ostream)
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'pos_x))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'pos_y))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'pos_z))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'qua_x))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'qua_y))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'qua_z))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'qua_w))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let* ((signed (cl:slot-value msg 'horizontal)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'vertical)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'temperature))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream))
   (cl:slot-value msg 'temperature))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'v_format))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'v_format))
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'v_data))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream))
   (cl:slot-value msg 'v_data))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <sync_data>) istream)
  "Deserializes a message object of type '<sync_data>"
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'stamp) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'stamp) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'version)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'device_id)) (cl:read-byte istream))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'pos_x) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'pos_y) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'pos_z) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'qua_x) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'qua_y) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'qua_z) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'qua_w) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'horizontal) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'vertical) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'temperature) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'temperature)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'v_format) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'v_format) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'v_data) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'v_data)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream)))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<sync_data>)))
  "Returns string type for a message object of type '<sync_data>"
  "fixed_msg/sync_data")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'sync_data)))
  "Returns string type for a message object of type 'sync_data"
  "fixed_msg/sync_data")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<sync_data>)))
  "Returns md5sum for a message object of type '<sync_data>"
  "7bc7618cef60033e2d78462cb5d33ffd")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'sync_data)))
  "Returns md5sum for a message object of type 'sync_data"
  "7bc7618cef60033e2d78462cb5d33ffd")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<sync_data>)))
  "Returns full string definition for message of type '<sync_data>"
  (cl:format cl:nil "string stamp~%uint8 version~%uint8 device_id~%float32 pos_x~%float32 pos_y~%float32 pos_z~%float32 qua_x~%float32 qua_y~%float32 qua_z~%float32 qua_w~%int32 horizontal~%int32 vertical~%uint8[] temperature~%string v_format~%uint8[] v_data~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'sync_data)))
  "Returns full string definition for message of type 'sync_data"
  (cl:format cl:nil "string stamp~%uint8 version~%uint8 device_id~%float32 pos_x~%float32 pos_y~%float32 pos_z~%float32 qua_x~%float32 qua_y~%float32 qua_z~%float32 qua_w~%int32 horizontal~%int32 vertical~%uint8[] temperature~%string v_format~%uint8[] v_data~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <sync_data>))
  (cl:+ 0
     4 (cl:length (cl:slot-value msg 'stamp))
     1
     1
     4
     4
     4
     4
     4
     4
     4
     4
     4
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'temperature) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
     4 (cl:length (cl:slot-value msg 'v_format))
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'v_data) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <sync_data>))
  "Converts a ROS message object to a list"
  (cl:list 'sync_data
    (cl:cons ':stamp (stamp msg))
    (cl:cons ':version (version msg))
    (cl:cons ':device_id (device_id msg))
    (cl:cons ':pos_x (pos_x msg))
    (cl:cons ':pos_y (pos_y msg))
    (cl:cons ':pos_z (pos_z msg))
    (cl:cons ':qua_x (qua_x msg))
    (cl:cons ':qua_y (qua_y msg))
    (cl:cons ':qua_z (qua_z msg))
    (cl:cons ':qua_w (qua_w msg))
    (cl:cons ':horizontal (horizontal msg))
    (cl:cons ':vertical (vertical msg))
    (cl:cons ':temperature (temperature msg))
    (cl:cons ':v_format (v_format msg))
    (cl:cons ':v_data (v_data msg))
))
