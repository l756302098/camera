; Auto-generated. Do not edit!


(cl:in-package fixed_msg-srv)


;//! \htmlinclude cp_control-request.msg.html

(cl:defclass <cp_control-request> (roslisp-msg-protocol:ros-message)
  ((id
    :reader id
    :initarg :id
    :type cl:integer
    :initform 0)
   (action
    :reader action
    :initarg :action
    :type cl:integer
    :initform 0)
   (type
    :reader type
    :initarg :type
    :type cl:integer
    :initform 0)
   (value
    :reader value
    :initarg :value
    :type cl:integer
    :initform 0)
   (allvalue
    :reader allvalue
    :initarg :allvalue
    :type (cl:vector cl:integer)
   :initform (cl:make-array 0 :element-type 'cl:integer :initial-element 0)))
)

(cl:defclass cp_control-request (<cp_control-request>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <cp_control-request>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'cp_control-request)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name fixed_msg-srv:<cp_control-request> is deprecated: use fixed_msg-srv:cp_control-request instead.")))

(cl:ensure-generic-function 'id-val :lambda-list '(m))
(cl:defmethod id-val ((m <cp_control-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-srv:id-val is deprecated.  Use fixed_msg-srv:id instead.")
  (id m))

(cl:ensure-generic-function 'action-val :lambda-list '(m))
(cl:defmethod action-val ((m <cp_control-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-srv:action-val is deprecated.  Use fixed_msg-srv:action instead.")
  (action m))

(cl:ensure-generic-function 'type-val :lambda-list '(m))
(cl:defmethod type-val ((m <cp_control-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-srv:type-val is deprecated.  Use fixed_msg-srv:type instead.")
  (type m))

(cl:ensure-generic-function 'value-val :lambda-list '(m))
(cl:defmethod value-val ((m <cp_control-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-srv:value-val is deprecated.  Use fixed_msg-srv:value instead.")
  (value m))

(cl:ensure-generic-function 'allvalue-val :lambda-list '(m))
(cl:defmethod allvalue-val ((m <cp_control-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-srv:allvalue-val is deprecated.  Use fixed_msg-srv:allvalue instead.")
  (allvalue m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <cp_control-request>) ostream)
  "Serializes a message object of type '<cp_control-request>"
  (cl:let* ((signed (cl:slot-value msg 'id)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'action)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'type)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'value)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'allvalue))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) ele) ostream))
   (cl:slot-value msg 'allvalue))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <cp_control-request>) istream)
  "Deserializes a message object of type '<cp_control-request>"
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'id) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'action) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'type) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'value) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'allvalue) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'allvalue)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:aref vals i)) (cl:read-byte istream)))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<cp_control-request>)))
  "Returns string type for a service object of type '<cp_control-request>"
  "fixed_msg/cp_controlRequest")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'cp_control-request)))
  "Returns string type for a service object of type 'cp_control-request"
  "fixed_msg/cp_controlRequest")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<cp_control-request>)))
  "Returns md5sum for a message object of type '<cp_control-request>"
  "be6cabd355b563d374071eb0d114fd3d")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'cp_control-request)))
  "Returns md5sum for a message object of type 'cp_control-request"
  "be6cabd355b563d374071eb0d114fd3d")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<cp_control-request>)))
  "Returns full string definition for message of type '<cp_control-request>"
  (cl:format cl:nil "int32 id~%int32 action~%int32 type~%int32 value~%uint32[] allvalue~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'cp_control-request)))
  "Returns full string definition for message of type 'cp_control-request"
  (cl:format cl:nil "int32 id~%int32 action~%int32 type~%int32 value~%uint32[] allvalue~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <cp_control-request>))
  (cl:+ 0
     4
     4
     4
     4
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'allvalue) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 4)))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <cp_control-request>))
  "Converts a ROS message object to a list"
  (cl:list 'cp_control-request
    (cl:cons ':id (id msg))
    (cl:cons ':action (action msg))
    (cl:cons ':type (type msg))
    (cl:cons ':value (value msg))
    (cl:cons ':allvalue (allvalue msg))
))
;//! \htmlinclude cp_control-response.msg.html

(cl:defclass <cp_control-response> (roslisp-msg-protocol:ros-message)
  ((result
    :reader result
    :initarg :result
    :type cl:integer
    :initform 0))
)

(cl:defclass cp_control-response (<cp_control-response>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <cp_control-response>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'cp_control-response)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name fixed_msg-srv:<cp_control-response> is deprecated: use fixed_msg-srv:cp_control-response instead.")))

(cl:ensure-generic-function 'result-val :lambda-list '(m))
(cl:defmethod result-val ((m <cp_control-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader fixed_msg-srv:result-val is deprecated.  Use fixed_msg-srv:result instead.")
  (result m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <cp_control-response>) ostream)
  "Serializes a message object of type '<cp_control-response>"
  (cl:let* ((signed (cl:slot-value msg 'result)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <cp_control-response>) istream)
  "Deserializes a message object of type '<cp_control-response>"
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'result) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<cp_control-response>)))
  "Returns string type for a service object of type '<cp_control-response>"
  "fixed_msg/cp_controlResponse")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'cp_control-response)))
  "Returns string type for a service object of type 'cp_control-response"
  "fixed_msg/cp_controlResponse")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<cp_control-response>)))
  "Returns md5sum for a message object of type '<cp_control-response>"
  "be6cabd355b563d374071eb0d114fd3d")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'cp_control-response)))
  "Returns md5sum for a message object of type 'cp_control-response"
  "be6cabd355b563d374071eb0d114fd3d")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<cp_control-response>)))
  "Returns full string definition for message of type '<cp_control-response>"
  (cl:format cl:nil "int32 result~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'cp_control-response)))
  "Returns full string definition for message of type 'cp_control-response"
  (cl:format cl:nil "int32 result~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <cp_control-response>))
  (cl:+ 0
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <cp_control-response>))
  "Converts a ROS message object to a list"
  (cl:list 'cp_control-response
    (cl:cons ':result (result msg))
))
(cl:defmethod roslisp-msg-protocol:service-request-type ((msg (cl:eql 'cp_control)))
  'cp_control-request)
(cl:defmethod roslisp-msg-protocol:service-response-type ((msg (cl:eql 'cp_control)))
  'cp_control-response)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'cp_control)))
  "Returns string type for a service object of type '<cp_control>"
  "fixed_msg/cp_control")