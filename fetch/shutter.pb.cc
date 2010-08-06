// Generated by the protocol buffer compiler.  DO NOT EDIT!

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "shutter.pb.h"
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace fetch {
namespace cfg {
namespace device {

namespace {

const ::google::protobuf::Descriptor* Shutter_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  Shutter_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_shutter_2eproto() {
  protobuf_AddDesc_shutter_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "shutter.proto");
  GOOGLE_CHECK(file != NULL);
  Shutter_descriptor_ = file->message_type(0);
  static const int Shutter_offsets_[3] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Shutter, open_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Shutter, closed_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Shutter, do_channel_),
  };
  Shutter_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      Shutter_descriptor_,
      Shutter::default_instance_,
      Shutter_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Shutter, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Shutter, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(Shutter));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_shutter_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    Shutter_descriptor_, &Shutter::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_shutter_2eproto() {
  delete Shutter::default_instance_;
  delete Shutter_reflection_;
}

void protobuf_AddDesc_shutter_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\rshutter.proto\022\020fetch.cfg.device\"T\n\007Shu"
    "tter\022\017\n\004open\030\001 \001(\r:\0011\022\021\n\006closed\030\002 \001(\r:\0010"
    "\022%\n\ndo_channel\030\003 \001(\t:\021/Dev1/port0/line0", 119);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "shutter.proto", &protobuf_RegisterTypes);
  Shutter::default_instance_ = new Shutter();
  Shutter::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_shutter_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_shutter_2eproto {
  StaticDescriptorInitializer_shutter_2eproto() {
    protobuf_AddDesc_shutter_2eproto();
  }
} static_descriptor_initializer_shutter_2eproto_;


// ===================================================================

const ::std::string Shutter::_default_do_channel_("/Dev1/port0/line0");
#ifndef _MSC_VER
const int Shutter::kOpenFieldNumber;
const int Shutter::kClosedFieldNumber;
const int Shutter::kDoChannelFieldNumber;
#endif  // !_MSC_VER

Shutter::Shutter()
  : ::google::protobuf::Message() {
  SharedCtor();
}

void Shutter::InitAsDefaultInstance() {
}

Shutter::Shutter(const Shutter& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void Shutter::SharedCtor() {
  _cached_size_ = 0;
  open_ = 1u;
  closed_ = 0u;
  do_channel_ = const_cast< ::std::string*>(&_default_do_channel_);
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

Shutter::~Shutter() {
  SharedDtor();
}

void Shutter::SharedDtor() {
  if (do_channel_ != &_default_do_channel_) {
    delete do_channel_;
  }
  if (this != default_instance_) {
  }
}

void Shutter::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* Shutter::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return Shutter_descriptor_;
}

const Shutter& Shutter::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_shutter_2eproto();  return *default_instance_;
}

Shutter* Shutter::default_instance_ = NULL;

Shutter* Shutter::New() const {
  return new Shutter;
}

void Shutter::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    open_ = 1u;
    closed_ = 0u;
    if (_has_bit(2)) {
      if (do_channel_ != &_default_do_channel_) {
        do_channel_->assign(_default_do_channel_);
      }
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool Shutter::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional uint32 open = 1 [default = 1];
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &open_)));
          _set_bit(0);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(16)) goto parse_closed;
        break;
      }
      
      // optional uint32 closed = 2 [default = 0];
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_closed:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &closed_)));
          _set_bit(1);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(26)) goto parse_do_channel;
        break;
      }
      
      // optional string do_channel = 3 [default = "/Dev1/port0/line0"];
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_do_channel:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_do_channel()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->do_channel().data(), this->do_channel().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }
      
      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void Shutter::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // optional uint32 open = 1 [default = 1];
  if (_has_bit(0)) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(1, this->open(), output);
  }
  
  // optional uint32 closed = 2 [default = 0];
  if (_has_bit(1)) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(2, this->closed(), output);
  }
  
  // optional string do_channel = 3 [default = "/Dev1/port0/line0"];
  if (_has_bit(2)) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->do_channel().data(), this->do_channel().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      3, this->do_channel(), output);
  }
  
  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* Shutter::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // optional uint32 open = 1 [default = 1];
  if (_has_bit(0)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(1, this->open(), target);
  }
  
  // optional uint32 closed = 2 [default = 0];
  if (_has_bit(1)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(2, this->closed(), target);
  }
  
  // optional string do_channel = 3 [default = "/Dev1/port0/line0"];
  if (_has_bit(2)) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->do_channel().data(), this->do_channel().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        3, this->do_channel(), target);
  }
  
  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int Shutter::ByteSize() const {
  int total_size = 0;
  
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // optional uint32 open = 1 [default = 1];
    if (has_open()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->open());
    }
    
    // optional uint32 closed = 2 [default = 0];
    if (has_closed()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->closed());
    }
    
    // optional string do_channel = 3 [default = "/Dev1/port0/line0"];
    if (has_do_channel()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->do_channel());
    }
    
  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void Shutter::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const Shutter* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const Shutter*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void Shutter::MergeFrom(const Shutter& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from._has_bit(0)) {
      set_open(from.open());
    }
    if (from._has_bit(1)) {
      set_closed(from.closed());
    }
    if (from._has_bit(2)) {
      set_do_channel(from.do_channel());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void Shutter::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void Shutter::CopyFrom(const Shutter& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Shutter::IsInitialized() const {
  
  return true;
}

void Shutter::Swap(Shutter* other) {
  if (other != this) {
    std::swap(open_, other->open_);
    std::swap(closed_, other->closed_);
    std::swap(do_channel_, other->do_channel_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata Shutter::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = Shutter_descriptor_;
  metadata.reflection = Shutter_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace device
}  // namespace cfg
}  // namespace fetch

// @@protoc_insertion_point(global_scope)
