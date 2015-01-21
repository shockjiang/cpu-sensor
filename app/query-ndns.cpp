/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014, Regents of the University of California.
 *
 * This file is part of NDNS (Named Data Networking Domain Name Service).
 * See AUTHORS.md for complete list of NDNS authors and contributors.
 *
 * NDNS is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NDNS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NDNS, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "validator.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/name.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/data.hpp>
#include <ndn-cxx/util/time.hpp>

#include <boost/bind.hpp>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>

#include <algorithm>

namespace ndn {
namespace ndns {

constexpr int NDNS_TYTP_TLV = 189;
constexpr int NDNS_RAW = 0; ///<  mean that MetaInfo does not contain NdnsType
constexpr int NDNS_RESP = 1; ///< response type means there are requested RR
constexpr int NDNS_NACK = 2; ///< no requested RR
constexpr int NDNS_AUTH = 3; ///< only has RR for detailed (longer) label

class NdnsShot : boost::noncopyable
{
public:
  NdnsShot(const Name interestName, Face& face, Validator& validator)
    : m_interestName(interestName)
    , m_face(face)
    , m_validator(validator)
    , m_hasError(false)
  {
  }

public:
  bool
  hasError() const
  {
    return m_hasError;
  }

  void
  run()
  {
    Interest interest = this->toInterest();
    std::cout << "[* <- *] express Interest: " << interest.getName() << std::endl;
    m_face.expressInterest(interest,
                           boost::bind (&NdnsShot::onData, this, _1, _2), // NDNS-NACK may return
                           // dynamic binding, if onData is override, bind to the new function
                           boost::bind (&NdnsShot::onTimeout, this, _1) //dynamic binding
                           );
    try {
      m_face.processEvents();
    }
    catch (std::exception& e) {
      std::cout << "Face fails to process events: " << e.what() << std::endl;
    }
  }

private:
  void
  onData(const Interest& interest, const Data& data)
  {
    std::cout << "get data: " << data.getName() << std::endl;
        m_validator.validate(data,
                       boost::bind(&NdnsShot::onDataValidated, this, _1),
                       boost::bind(&NdnsShot::onDataValidationFailed, this, _1, _2)
                       );
    // m_validator.validate(data);
  }


  void
  onTimeout(const ndn::Interest& interest)
  {

    std::cerr << "[* !! *] Interest: " << interest.getName()
              <<" cannot fetch data" << std::endl;
    this->stop();
  }


  void
  onDataValidated(const shared_ptr<const Data>& data)
  {
    std::cout << "final data pass verification" << std::endl;
    const Block* block = data->getMetaInfo().findAppMetaInfo(NDNS_TYTP_TLV);
    if (block != nullptr) {
      int val = readNonNegativeInteger(*block);
      if (val == NDNS_RESP)
        std::cout << "get the final response" << std::endl;
      else if (val == NDNS_RAW)
        std::cout << "get NDNS_RAW. not standard RR. " << std::endl;
      else if (val == NDNS_NACK || val == NDNS_AUTH)
        std::cout << "get NDNS_NACK. The data does not exist" << std::endl;
      else
        std::cout << "unknown NdnsType:" << val << std::endl;
    }
    else
      std::cout << "get NDNS_RAW. not standard RR. " << std::endl;

    this->stop();
  }

  void
  onDataValidationFailed(const shared_ptr<const Data>& data, const std::string& str)
  {
    std::cout << "final data does not pass verification" << std::endl;
    m_hasError = true;
    this->stop();
  }

  Interest
  toInterest()
  {
    Interest interest(m_interestName);
    return interest;
  }

  void
  stop()
  {
    m_face.getIoService().stop();
    std::cout << "application stops." << std::endl;
  }


private:
  Name m_interestName;
  Face& m_face;
  Validator& m_validator;
  bool m_hasError;
}; // class NdnsShot

} // namespace ndns
} // namespace ndn


int main(int argc, char* argv[])
{
  using std::string;
  using namespace ndn;
  using namespace ndn::ndns;

  if (argc < 2) {
    std::cerr << "A NDNS Query name should be specified" << std::endl
              << "e.g.: demo /ndn/edu/ucla/NDNS/cs/shock/TXT" << std::endl;
    return 1;
  }
  Name name(argv[1]);
  Name zone;

  shared_ptr<Regex> regex = make_shared<Regex>("(<>*)<KEY>(<>+)<ID-CERT><>*");
  shared_ptr<Regex> regex2 = make_shared<Regex>("(<>*)<NDNS>(<>+)");

  if (regex->match(name)) {
    zone = regex->expand("\\1");
  }
  else if (regex2->match(name)) {
    zone = regex2->expand("\\1");
  }

  if (zone.empty()) {
    std::cerr << "The name: " << name << " does not contains NDNS tag: "
              << "NDNS or KEY. "
              << "Ingore name: " << name << std::endl;
  }
  else {
    std::cout << "The record stores at zone " << zone << std::endl;
  }

  Face face;
  ndns::Validator validator(face);

  NdnsShot shot(name, face, validator);

  shot.run();
  if (shot.hasError())
    return 1;
  else
    return 0;

  std::cout << (shot.hasError() ? "Fail" : "Succeed") << " to get Data of name " << name
            << std::endl;
  return 0;
}
