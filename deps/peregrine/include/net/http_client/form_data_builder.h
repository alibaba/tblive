#ifndef NET_FORM_DATA_BUILDER_H_
#define NET_FORM_DATA_BUILDER_H_
#include "base/rand_util.h"
#include <string>

namespace net {

class FormDataBuilder {
 public:
  FormDataBuilder(std::string& data,
    const std::string& boundary)
  : data_(data),
   boundary_(boundary) {
  }

  void addValueToMultiPart(const std::string& name,
    const std::string&  value,
    const std::string& content_type) {
    data_.append("--" + boundary_ + "\r\n");
    data_.append("Content-Disposition: form-data; name=\"" + name + "\"\r\n");
    if (!content_type.empty()) {
      data_.append("Content-Type: " + content_type + "\r\n");
    }
    data_.append("\r\n" + value + "\r\n");
    }
  void addFilenameToMultiPart(
    const std::string& file_name,
    const std::string& name,
    const std::string& value,
    const std::string& content_type) {
    data_.append("--" + boundary_ + "\r\n");
    data_.append("Content-Disposition: form-data; name=\"" + name + "\"; filename=\"" + file_name + "\"\r\n");
    if (!content_type.empty()) {
      data_.append("Content-Type: " + content_type + "\r\n");
    }
    data_.append("\r\n");
    data_.append(value);
    data_.append("\r\n");
  }
  void FinishMultiPart() {
    data_.append("--" + boundary_ + "--\r\n");
  }

  static std::string generateUniqueBoundaryString() {
    std::string boundary("----WebKitFormBoundary");
    static const char alphaNumericEncodingMap[64] = {
      0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
      0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
      0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
      0x59, 0x5A, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
      0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E,
      0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
      0x77, 0x78, 0x79, 0x7A, 0x30, 0x31, 0x32, 0x33,
      0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42
    };

    std::string randomBytes;

    for (unsigned i = 0; i < 4; ++i) {
      unsigned randomness = static_cast<unsigned>(base::RandDouble()* (std::numeric_limits<unsigned>::max() + 1.0));
      boundary.append(1, alphaNumericEncodingMap[(randomness >> 24) & 0x3F]);
      boundary.append(1, alphaNumericEncodingMap[(randomness >> 16) & 0x3F]);
      boundary.append(1, alphaNumericEncodingMap[(randomness >> 8) & 0x3F]);
      boundary.append(1, alphaNumericEncodingMap[randomness & 0x3F]);
    }
    return boundary;
  }

 private:
   std::string& data_;
   std::string boundary_;
};

}

#endif  // NET_FORM_DATA_BUILDER_H_
