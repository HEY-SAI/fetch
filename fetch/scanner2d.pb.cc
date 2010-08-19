// Generated by the protocol buffer compiler.  DO NOT EDIT!

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "scanner2d.pb.h"
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

const ::google::protobuf::Descriptor* Scanner2D_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  Scanner2D_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_scanner2d_2eproto() {
  protobuf_AddDesc_scanner2d_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "scanner2d.proto");
  GOOGLE_CHECK(file != NULL);
  Scanner2D_descriptor_ = file->message_type(0);
  static const int Scanner2D_offsets_[14] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, frequency_hz_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, nscans_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, line_duty_cycle_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, line_trigger_src_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, trigger_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, armstart_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, clock_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, ctr_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, ctr_alt_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, ao_samples_per_frame_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, digitizer_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, pockels_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, shutter_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, linear_scan_mirror_),
  };
  Scanner2D_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      Scanner2D_descriptor_,
      Scanner2D::default_instance_,
      Scanner2D_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Scanner2D, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(Scanner2D));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_scanner2d_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    Scanner2D_descriptor_, &Scanner2D::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_scanner2d_2eproto() {
  delete Scanner2D::default_instance_;
  delete Scanner2D_reflection_;
}

void protobuf_AddDesc_scanner2d_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::fetch::cfg::device::protobuf_AddDesc_digitizer_2eproto();
  ::fetch::cfg::device::protobuf_AddDesc_pockels_2eproto();
  ::fetch::cfg::device::protobuf_AddDesc_shutter_2eproto();
  ::fetch::cfg::device::protobuf_AddDesc_linear_5fscan_5fmirror_2eproto();
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\017scanner2d.proto\022\020fetch.cfg.device\032\017dig"
    "itizer.proto\032\rpockels.proto\032\rshutter.pro"
    "to\032\030linear_scan_mirror.proto\"\357\003\n\tScanner"
    "2D\022\032\n\014frequency_hz\030\001 \001(\001:\0047920\022\023\n\006nscans"
    "\030\002 \001(\r:\003512\022\035\n\017line_duty_cycle\030\003 \001(\002:\0040."
    "95\022\033\n\020line_trigger_src\030\004 \001(\r:\0011\022\026\n\007trigg"
    "er\030\005 \001(\t:\005APFI0\022\027\n\010armstart\030\006 \001(\t:\005RTSI2"
    "\022!\n\005clock\030\007 \001(\t:\022Ctr1InternalOutput\022\027\n\003c"
    "tr\030\010 \001(\t:\n/Dev1/ctr1\022\033\n\007ctr_alt\030\t \001(\t:\n/"
    "Dev1/ctr0\022#\n\024ao_samples_per_frame\030\n \001(\r:"
    "\00516384\022.\n\tdigitizer\030\013 \001(\0132\033.fetch.cfg.de"
    "vice.Digitizer\022*\n\007pockels\030\014 \001(\0132\031.fetch."
    "cfg.device.Pockels\022*\n\007shutter\030\r \001(\0132\031.fe"
    "tch.cfg.device.Shutter\022>\n\022linear_scan_mi"
    "rror\030\016 \001(\0132\".fetch.cfg.device.LinearScan"
    "Mirror", 606);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "scanner2d.proto", &protobuf_RegisterTypes);
  Scanner2D::default_instance_ = new Scanner2D();
  Scanner2D::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_scanner2d_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_scanner2d_2eproto {
  StaticDescriptorInitializer_scanner2d_2eproto() {
    protobuf_AddDesc_scanner2d_2eproto();
  }
} static_descriptor_initializer_scanner2d_2eproto_;


// ===================================================================

const ::std::string Scanner2D::_default_trigger_("APFI0");
const ::std::string Scanner2D::_default_armstart_("RTSI2");
const ::std::string Scanner2D::_default_clock_("Ctr1InternalOutput");
const ::std::string Scanner2D::_default_ctr_("/Dev1/ctr1");
const ::std::string Scanner2D::_default_ctr_alt_("/Dev1/ctr0");
#ifndef _MSC_VER
const int Scanner2D::kFrequencyHzFieldNumber;
const int Scanner2D::kNscansFieldNumber;
const int Scanner2D::kLineDutyCycleFieldNumber;
const int Scanner2D::kLineTriggerSrcFieldNumber;
const int Scanner2D::kTriggerFieldNumber;
const int Scanner2D::kArmstartFieldNumber;
const int Scanner2D::kClockFieldNumber;
const int Scanner2D::kCtrFieldNumber;
const int Scanner2D::kCtrAltFieldNumber;
const int Scanner2D::kAoSamplesPerFrameFieldNumber;
const int Scanner2D::kDigitizerFieldNumber;
const int Scanner2D::kPockelsFieldNumber;
const int Scanner2D::kShutterFieldNumber;
const int Scanner2D::kLinearScanMirrorFieldNumber;
#endif  // !_MSC_VER

Scanner2D::Scanner2D()
  : ::google::protobuf::Message() {
  SharedCtor();
}

void Scanner2D::InitAsDefaultInstance() {
  digitizer_ = const_cast< ::fetch::cfg::device::Digitizer*>(&::fetch::cfg::device::Digitizer::default_instance());
  pockels_ = const_cast< ::fetch::cfg::device::Pockels*>(&::fetch::cfg::device::Pockels::default_instance());
  shutter_ = const_cast< ::fetch::cfg::device::Shutter*>(&::fetch::cfg::device::Shutter::default_instance());
  linear_scan_mirror_ = const_cast< ::fetch::cfg::device::LinearScanMirror*>(&::fetch::cfg::device::LinearScanMirror::default_instance());
}

Scanner2D::Scanner2D(const Scanner2D& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void Scanner2D::SharedCtor() {
  _cached_size_ = 0;
  frequency_hz_ = 7920;
  nscans_ = 512u;
  line_duty_cycle_ = 0.95f;
  line_trigger_src_ = 1u;
  trigger_ = const_cast< ::std::string*>(&_default_trigger_);
  armstart_ = const_cast< ::std::string*>(&_default_armstart_);
  clock_ = const_cast< ::std::string*>(&_default_clock_);
  ctr_ = const_cast< ::std::string*>(&_default_ctr_);
  ctr_alt_ = const_cast< ::std::string*>(&_default_ctr_alt_);
  ao_samples_per_frame_ = 16384u;
  digitizer_ = NULL;
  pockels_ = NULL;
  shutter_ = NULL;
  linear_scan_mirror_ = NULL;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

Scanner2D::~Scanner2D() {
  SharedDtor();
}

void Scanner2D::SharedDtor() {
  if (trigger_ != &_default_trigger_) {
    delete trigger_;
  }
  if (armstart_ != &_default_armstart_) {
    delete armstart_;
  }
  if (clock_ != &_default_clock_) {
    delete clock_;
  }
  if (ctr_ != &_default_ctr_) {
    delete ctr_;
  }
  if (ctr_alt_ != &_default_ctr_alt_) {
    delete ctr_alt_;
  }
  if (this != default_instance_) {
    delete digitizer_;
    delete pockels_;
    delete shutter_;
    delete linear_scan_mirror_;
  }
}

void Scanner2D::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* Scanner2D::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return Scanner2D_descriptor_;
}

const Scanner2D& Scanner2D::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_scanner2d_2eproto();  return *default_instance_;
}

Scanner2D* Scanner2D::default_instance_ = NULL;

Scanner2D* Scanner2D::New() const {
  return new Scanner2D;
}

void Scanner2D::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    frequency_hz_ = 7920;
    nscans_ = 512u;
    line_duty_cycle_ = 0.95f;
    line_trigger_src_ = 1u;
    if (_has_bit(4)) {
      if (trigger_ != &_default_trigger_) {
        trigger_->assign(_default_trigger_);
      }
    }
    if (_has_bit(5)) {
      if (armstart_ != &_default_armstart_) {
        armstart_->assign(_default_armstart_);
      }
    }
    if (_has_bit(6)) {
      if (clock_ != &_default_clock_) {
        clock_->assign(_default_clock_);
      }
    }
    if (_has_bit(7)) {
      if (ctr_ != &_default_ctr_) {
        ctr_->assign(_default_ctr_);
      }
    }
  }
  if (_has_bits_[8 / 32] & (0xffu << (8 % 32))) {
    if (_has_bit(8)) {
      if (ctr_alt_ != &_default_ctr_alt_) {
        ctr_alt_->assign(_default_ctr_alt_);
      }
    }
    ao_samples_per_frame_ = 16384u;
    if (_has_bit(10)) {
      if (digitizer_ != NULL) digitizer_->::fetch::cfg::device::Digitizer::Clear();
    }
    if (_has_bit(11)) {
      if (pockels_ != NULL) pockels_->::fetch::cfg::device::Pockels::Clear();
    }
    if (_has_bit(12)) {
      if (shutter_ != NULL) shutter_->::fetch::cfg::device::Shutter::Clear();
    }
    if (_has_bit(13)) {
      if (linear_scan_mirror_ != NULL) linear_scan_mirror_->::fetch::cfg::device::LinearScanMirror::Clear();
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool Scanner2D::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional double frequency_hz = 1 [default = 7920];
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   double, ::google::protobuf::internal::WireFormatLite::TYPE_DOUBLE>(
                 input, &frequency_hz_)));
          _set_bit(0);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(16)) goto parse_nscans;
        break;
      }
      
      // optional uint32 nscans = 2 [default = 512];
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_nscans:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &nscans_)));
          _set_bit(1);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(29)) goto parse_line_duty_cycle;
        break;
      }
      
      // optional float line_duty_cycle = 3 [default = 0.95];
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED32) {
         parse_line_duty_cycle:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   float, ::google::protobuf::internal::WireFormatLite::TYPE_FLOAT>(
                 input, &line_duty_cycle_)));
          _set_bit(2);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(32)) goto parse_line_trigger_src;
        break;
      }
      
      // optional uint32 line_trigger_src = 4 [default = 1];
      case 4: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_line_trigger_src:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &line_trigger_src_)));
          _set_bit(3);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(42)) goto parse_trigger;
        break;
      }
      
      // optional string trigger = 5 [default = "APFI0"];
      case 5: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_trigger:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_trigger()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->trigger().data(), this->trigger().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(50)) goto parse_armstart;
        break;
      }
      
      // optional string armstart = 6 [default = "RTSI2"];
      case 6: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_armstart:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_armstart()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->armstart().data(), this->armstart().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(58)) goto parse_clock;
        break;
      }
      
      // optional string clock = 7 [default = "Ctr1InternalOutput"];
      case 7: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_clock:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_clock()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->clock().data(), this->clock().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(66)) goto parse_ctr;
        break;
      }
      
      // optional string ctr = 8 [default = "/Dev1/ctr1"];
      case 8: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_ctr:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_ctr()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->ctr().data(), this->ctr().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(74)) goto parse_ctr_alt;
        break;
      }
      
      // optional string ctr_alt = 9 [default = "/Dev1/ctr0"];
      case 9: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_ctr_alt:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_ctr_alt()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->ctr_alt().data(), this->ctr_alt().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(80)) goto parse_ao_samples_per_frame;
        break;
      }
      
      // optional uint32 ao_samples_per_frame = 10 [default = 16384];
      case 10: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_ao_samples_per_frame:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &ao_samples_per_frame_)));
          _set_bit(9);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(90)) goto parse_digitizer;
        break;
      }
      
      // optional .fetch.cfg.device.Digitizer digitizer = 11;
      case 11: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_digitizer:
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
               input, mutable_digitizer()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(98)) goto parse_pockels;
        break;
      }
      
      // optional .fetch.cfg.device.Pockels pockels = 12;
      case 12: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_pockels:
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
               input, mutable_pockels()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(106)) goto parse_shutter;
        break;
      }
      
      // optional .fetch.cfg.device.Shutter shutter = 13;
      case 13: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_shutter:
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
               input, mutable_shutter()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(114)) goto parse_linear_scan_mirror;
        break;
      }
      
      // optional .fetch.cfg.device.LinearScanMirror linear_scan_mirror = 14;
      case 14: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_linear_scan_mirror:
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
               input, mutable_linear_scan_mirror()));
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

void Scanner2D::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // optional double frequency_hz = 1 [default = 7920];
  if (_has_bit(0)) {
    ::google::protobuf::internal::WireFormatLite::WriteDouble(1, this->frequency_hz(), output);
  }
  
  // optional uint32 nscans = 2 [default = 512];
  if (_has_bit(1)) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(2, this->nscans(), output);
  }
  
  // optional float line_duty_cycle = 3 [default = 0.95];
  if (_has_bit(2)) {
    ::google::protobuf::internal::WireFormatLite::WriteFloat(3, this->line_duty_cycle(), output);
  }
  
  // optional uint32 line_trigger_src = 4 [default = 1];
  if (_has_bit(3)) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(4, this->line_trigger_src(), output);
  }
  
  // optional string trigger = 5 [default = "APFI0"];
  if (_has_bit(4)) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->trigger().data(), this->trigger().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      5, this->trigger(), output);
  }
  
  // optional string armstart = 6 [default = "RTSI2"];
  if (_has_bit(5)) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->armstart().data(), this->armstart().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      6, this->armstart(), output);
  }
  
  // optional string clock = 7 [default = "Ctr1InternalOutput"];
  if (_has_bit(6)) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->clock().data(), this->clock().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      7, this->clock(), output);
  }
  
  // optional string ctr = 8 [default = "/Dev1/ctr1"];
  if (_has_bit(7)) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->ctr().data(), this->ctr().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      8, this->ctr(), output);
  }
  
  // optional string ctr_alt = 9 [default = "/Dev1/ctr0"];
  if (_has_bit(8)) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->ctr_alt().data(), this->ctr_alt().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      9, this->ctr_alt(), output);
  }
  
  // optional uint32 ao_samples_per_frame = 10 [default = 16384];
  if (_has_bit(9)) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(10, this->ao_samples_per_frame(), output);
  }
  
  // optional .fetch.cfg.device.Digitizer digitizer = 11;
  if (_has_bit(10)) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      11, this->digitizer(), output);
  }
  
  // optional .fetch.cfg.device.Pockels pockels = 12;
  if (_has_bit(11)) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      12, this->pockels(), output);
  }
  
  // optional .fetch.cfg.device.Shutter shutter = 13;
  if (_has_bit(12)) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      13, this->shutter(), output);
  }
  
  // optional .fetch.cfg.device.LinearScanMirror linear_scan_mirror = 14;
  if (_has_bit(13)) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      14, this->linear_scan_mirror(), output);
  }
  
  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* Scanner2D::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // optional double frequency_hz = 1 [default = 7920];
  if (_has_bit(0)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteDoubleToArray(1, this->frequency_hz(), target);
  }
  
  // optional uint32 nscans = 2 [default = 512];
  if (_has_bit(1)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(2, this->nscans(), target);
  }
  
  // optional float line_duty_cycle = 3 [default = 0.95];
  if (_has_bit(2)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteFloatToArray(3, this->line_duty_cycle(), target);
  }
  
  // optional uint32 line_trigger_src = 4 [default = 1];
  if (_has_bit(3)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(4, this->line_trigger_src(), target);
  }
  
  // optional string trigger = 5 [default = "APFI0"];
  if (_has_bit(4)) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->trigger().data(), this->trigger().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        5, this->trigger(), target);
  }
  
  // optional string armstart = 6 [default = "RTSI2"];
  if (_has_bit(5)) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->armstart().data(), this->armstart().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        6, this->armstart(), target);
  }
  
  // optional string clock = 7 [default = "Ctr1InternalOutput"];
  if (_has_bit(6)) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->clock().data(), this->clock().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        7, this->clock(), target);
  }
  
  // optional string ctr = 8 [default = "/Dev1/ctr1"];
  if (_has_bit(7)) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->ctr().data(), this->ctr().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        8, this->ctr(), target);
  }
  
  // optional string ctr_alt = 9 [default = "/Dev1/ctr0"];
  if (_has_bit(8)) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->ctr_alt().data(), this->ctr_alt().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        9, this->ctr_alt(), target);
  }
  
  // optional uint32 ao_samples_per_frame = 10 [default = 16384];
  if (_has_bit(9)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(10, this->ao_samples_per_frame(), target);
  }
  
  // optional .fetch.cfg.device.Digitizer digitizer = 11;
  if (_has_bit(10)) {
    target = ::google::protobuf::internal::WireFormatLite::
      WriteMessageNoVirtualToArray(
        11, this->digitizer(), target);
  }
  
  // optional .fetch.cfg.device.Pockels pockels = 12;
  if (_has_bit(11)) {
    target = ::google::protobuf::internal::WireFormatLite::
      WriteMessageNoVirtualToArray(
        12, this->pockels(), target);
  }
  
  // optional .fetch.cfg.device.Shutter shutter = 13;
  if (_has_bit(12)) {
    target = ::google::protobuf::internal::WireFormatLite::
      WriteMessageNoVirtualToArray(
        13, this->shutter(), target);
  }
  
  // optional .fetch.cfg.device.LinearScanMirror linear_scan_mirror = 14;
  if (_has_bit(13)) {
    target = ::google::protobuf::internal::WireFormatLite::
      WriteMessageNoVirtualToArray(
        14, this->linear_scan_mirror(), target);
  }
  
  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int Scanner2D::ByteSize() const {
  int total_size = 0;
  
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // optional double frequency_hz = 1 [default = 7920];
    if (has_frequency_hz()) {
      total_size += 1 + 8;
    }
    
    // optional uint32 nscans = 2 [default = 512];
    if (has_nscans()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->nscans());
    }
    
    // optional float line_duty_cycle = 3 [default = 0.95];
    if (has_line_duty_cycle()) {
      total_size += 1 + 4;
    }
    
    // optional uint32 line_trigger_src = 4 [default = 1];
    if (has_line_trigger_src()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->line_trigger_src());
    }
    
    // optional string trigger = 5 [default = "APFI0"];
    if (has_trigger()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->trigger());
    }
    
    // optional string armstart = 6 [default = "RTSI2"];
    if (has_armstart()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->armstart());
    }
    
    // optional string clock = 7 [default = "Ctr1InternalOutput"];
    if (has_clock()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->clock());
    }
    
    // optional string ctr = 8 [default = "/Dev1/ctr1"];
    if (has_ctr()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->ctr());
    }
    
  }
  if (_has_bits_[8 / 32] & (0xffu << (8 % 32))) {
    // optional string ctr_alt = 9 [default = "/Dev1/ctr0"];
    if (has_ctr_alt()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->ctr_alt());
    }
    
    // optional uint32 ao_samples_per_frame = 10 [default = 16384];
    if (has_ao_samples_per_frame()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->ao_samples_per_frame());
    }
    
    // optional .fetch.cfg.device.Digitizer digitizer = 11;
    if (has_digitizer()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
          this->digitizer());
    }
    
    // optional .fetch.cfg.device.Pockels pockels = 12;
    if (has_pockels()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
          this->pockels());
    }
    
    // optional .fetch.cfg.device.Shutter shutter = 13;
    if (has_shutter()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
          this->shutter());
    }
    
    // optional .fetch.cfg.device.LinearScanMirror linear_scan_mirror = 14;
    if (has_linear_scan_mirror()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
          this->linear_scan_mirror());
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

void Scanner2D::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const Scanner2D* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const Scanner2D*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void Scanner2D::MergeFrom(const Scanner2D& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from._has_bit(0)) {
      set_frequency_hz(from.frequency_hz());
    }
    if (from._has_bit(1)) {
      set_nscans(from.nscans());
    }
    if (from._has_bit(2)) {
      set_line_duty_cycle(from.line_duty_cycle());
    }
    if (from._has_bit(3)) {
      set_line_trigger_src(from.line_trigger_src());
    }
    if (from._has_bit(4)) {
      set_trigger(from.trigger());
    }
    if (from._has_bit(5)) {
      set_armstart(from.armstart());
    }
    if (from._has_bit(6)) {
      set_clock(from.clock());
    }
    if (from._has_bit(7)) {
      set_ctr(from.ctr());
    }
  }
  if (from._has_bits_[8 / 32] & (0xffu << (8 % 32))) {
    if (from._has_bit(8)) {
      set_ctr_alt(from.ctr_alt());
    }
    if (from._has_bit(9)) {
      set_ao_samples_per_frame(from.ao_samples_per_frame());
    }
    if (from._has_bit(10)) {
      mutable_digitizer()->::fetch::cfg::device::Digitizer::MergeFrom(from.digitizer());
    }
    if (from._has_bit(11)) {
      mutable_pockels()->::fetch::cfg::device::Pockels::MergeFrom(from.pockels());
    }
    if (from._has_bit(12)) {
      mutable_shutter()->::fetch::cfg::device::Shutter::MergeFrom(from.shutter());
    }
    if (from._has_bit(13)) {
      mutable_linear_scan_mirror()->::fetch::cfg::device::LinearScanMirror::MergeFrom(from.linear_scan_mirror());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void Scanner2D::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void Scanner2D::CopyFrom(const Scanner2D& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Scanner2D::IsInitialized() const {
  
  return true;
}

void Scanner2D::Swap(Scanner2D* other) {
  if (other != this) {
    std::swap(frequency_hz_, other->frequency_hz_);
    std::swap(nscans_, other->nscans_);
    std::swap(line_duty_cycle_, other->line_duty_cycle_);
    std::swap(line_trigger_src_, other->line_trigger_src_);
    std::swap(trigger_, other->trigger_);
    std::swap(armstart_, other->armstart_);
    std::swap(clock_, other->clock_);
    std::swap(ctr_, other->ctr_);
    std::swap(ctr_alt_, other->ctr_alt_);
    std::swap(ao_samples_per_frame_, other->ao_samples_per_frame_);
    std::swap(digitizer_, other->digitizer_);
    std::swap(pockels_, other->pockels_);
    std::swap(shutter_, other->shutter_);
    std::swap(linear_scan_mirror_, other->linear_scan_mirror_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata Scanner2D::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = Scanner2D_descriptor_;
  metadata.reflection = Scanner2D_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace device
}  // namespace cfg
}  // namespace fetch

// @@protoc_insertion_point(global_scope)
