// Auto-generated. Do not edit!

// (in-package fixed_msg.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;

//-----------------------------------------------------------

class sync_data {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.stamp = null;
      this.version = null;
      this.device_id = null;
      this.pos_x = null;
      this.pos_y = null;
      this.pos_z = null;
      this.qua_x = null;
      this.qua_y = null;
      this.qua_z = null;
      this.qua_w = null;
      this.horizontal = null;
      this.vertical = null;
      this.temperature = null;
      this.v_format = null;
      this.v_data = null;
    }
    else {
      if (initObj.hasOwnProperty('stamp')) {
        this.stamp = initObj.stamp
      }
      else {
        this.stamp = '';
      }
      if (initObj.hasOwnProperty('version')) {
        this.version = initObj.version
      }
      else {
        this.version = 0;
      }
      if (initObj.hasOwnProperty('device_id')) {
        this.device_id = initObj.device_id
      }
      else {
        this.device_id = 0;
      }
      if (initObj.hasOwnProperty('pos_x')) {
        this.pos_x = initObj.pos_x
      }
      else {
        this.pos_x = 0.0;
      }
      if (initObj.hasOwnProperty('pos_y')) {
        this.pos_y = initObj.pos_y
      }
      else {
        this.pos_y = 0.0;
      }
      if (initObj.hasOwnProperty('pos_z')) {
        this.pos_z = initObj.pos_z
      }
      else {
        this.pos_z = 0.0;
      }
      if (initObj.hasOwnProperty('qua_x')) {
        this.qua_x = initObj.qua_x
      }
      else {
        this.qua_x = 0.0;
      }
      if (initObj.hasOwnProperty('qua_y')) {
        this.qua_y = initObj.qua_y
      }
      else {
        this.qua_y = 0.0;
      }
      if (initObj.hasOwnProperty('qua_z')) {
        this.qua_z = initObj.qua_z
      }
      else {
        this.qua_z = 0.0;
      }
      if (initObj.hasOwnProperty('qua_w')) {
        this.qua_w = initObj.qua_w
      }
      else {
        this.qua_w = 0.0;
      }
      if (initObj.hasOwnProperty('horizontal')) {
        this.horizontal = initObj.horizontal
      }
      else {
        this.horizontal = 0;
      }
      if (initObj.hasOwnProperty('vertical')) {
        this.vertical = initObj.vertical
      }
      else {
        this.vertical = 0;
      }
      if (initObj.hasOwnProperty('temperature')) {
        this.temperature = initObj.temperature
      }
      else {
        this.temperature = [];
      }
      if (initObj.hasOwnProperty('v_format')) {
        this.v_format = initObj.v_format
      }
      else {
        this.v_format = '';
      }
      if (initObj.hasOwnProperty('v_data')) {
        this.v_data = initObj.v_data
      }
      else {
        this.v_data = [];
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type sync_data
    // Serialize message field [stamp]
    bufferOffset = _serializer.string(obj.stamp, buffer, bufferOffset);
    // Serialize message field [version]
    bufferOffset = _serializer.uint8(obj.version, buffer, bufferOffset);
    // Serialize message field [device_id]
    bufferOffset = _serializer.uint8(obj.device_id, buffer, bufferOffset);
    // Serialize message field [pos_x]
    bufferOffset = _serializer.float32(obj.pos_x, buffer, bufferOffset);
    // Serialize message field [pos_y]
    bufferOffset = _serializer.float32(obj.pos_y, buffer, bufferOffset);
    // Serialize message field [pos_z]
    bufferOffset = _serializer.float32(obj.pos_z, buffer, bufferOffset);
    // Serialize message field [qua_x]
    bufferOffset = _serializer.float32(obj.qua_x, buffer, bufferOffset);
    // Serialize message field [qua_y]
    bufferOffset = _serializer.float32(obj.qua_y, buffer, bufferOffset);
    // Serialize message field [qua_z]
    bufferOffset = _serializer.float32(obj.qua_z, buffer, bufferOffset);
    // Serialize message field [qua_w]
    bufferOffset = _serializer.float32(obj.qua_w, buffer, bufferOffset);
    // Serialize message field [horizontal]
    bufferOffset = _serializer.int32(obj.horizontal, buffer, bufferOffset);
    // Serialize message field [vertical]
    bufferOffset = _serializer.int32(obj.vertical, buffer, bufferOffset);
    // Serialize message field [temperature]
    bufferOffset = _arraySerializer.uint8(obj.temperature, buffer, bufferOffset, null);
    // Serialize message field [v_format]
    bufferOffset = _serializer.string(obj.v_format, buffer, bufferOffset);
    // Serialize message field [v_data]
    bufferOffset = _arraySerializer.uint8(obj.v_data, buffer, bufferOffset, null);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type sync_data
    let len;
    let data = new sync_data(null);
    // Deserialize message field [stamp]
    data.stamp = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [version]
    data.version = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [device_id]
    data.device_id = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [pos_x]
    data.pos_x = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [pos_y]
    data.pos_y = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [pos_z]
    data.pos_z = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [qua_x]
    data.qua_x = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [qua_y]
    data.qua_y = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [qua_z]
    data.qua_z = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [qua_w]
    data.qua_w = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [horizontal]
    data.horizontal = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [vertical]
    data.vertical = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [temperature]
    data.temperature = _arrayDeserializer.uint8(buffer, bufferOffset, null)
    // Deserialize message field [v_format]
    data.v_format = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [v_data]
    data.v_data = _arrayDeserializer.uint8(buffer, bufferOffset, null)
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += object.stamp.length;
    length += object.temperature.length;
    length += object.v_format.length;
    length += object.v_data.length;
    return length + 54;
  }

  static datatype() {
    // Returns string type for a message object
    return 'fixed_msg/sync_data';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '7bc7618cef60033e2d78462cb5d33ffd';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    string stamp
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
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new sync_data(null);
    if (msg.stamp !== undefined) {
      resolved.stamp = msg.stamp;
    }
    else {
      resolved.stamp = ''
    }

    if (msg.version !== undefined) {
      resolved.version = msg.version;
    }
    else {
      resolved.version = 0
    }

    if (msg.device_id !== undefined) {
      resolved.device_id = msg.device_id;
    }
    else {
      resolved.device_id = 0
    }

    if (msg.pos_x !== undefined) {
      resolved.pos_x = msg.pos_x;
    }
    else {
      resolved.pos_x = 0.0
    }

    if (msg.pos_y !== undefined) {
      resolved.pos_y = msg.pos_y;
    }
    else {
      resolved.pos_y = 0.0
    }

    if (msg.pos_z !== undefined) {
      resolved.pos_z = msg.pos_z;
    }
    else {
      resolved.pos_z = 0.0
    }

    if (msg.qua_x !== undefined) {
      resolved.qua_x = msg.qua_x;
    }
    else {
      resolved.qua_x = 0.0
    }

    if (msg.qua_y !== undefined) {
      resolved.qua_y = msg.qua_y;
    }
    else {
      resolved.qua_y = 0.0
    }

    if (msg.qua_z !== undefined) {
      resolved.qua_z = msg.qua_z;
    }
    else {
      resolved.qua_z = 0.0
    }

    if (msg.qua_w !== undefined) {
      resolved.qua_w = msg.qua_w;
    }
    else {
      resolved.qua_w = 0.0
    }

    if (msg.horizontal !== undefined) {
      resolved.horizontal = msg.horizontal;
    }
    else {
      resolved.horizontal = 0
    }

    if (msg.vertical !== undefined) {
      resolved.vertical = msg.vertical;
    }
    else {
      resolved.vertical = 0
    }

    if (msg.temperature !== undefined) {
      resolved.temperature = msg.temperature;
    }
    else {
      resolved.temperature = []
    }

    if (msg.v_format !== undefined) {
      resolved.v_format = msg.v_format;
    }
    else {
      resolved.v_format = ''
    }

    if (msg.v_data !== undefined) {
      resolved.v_data = msg.v_data;
    }
    else {
      resolved.v_data = []
    }

    return resolved;
    }
};

module.exports = sync_data;
