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

/*
  #include "clients/response.hpp"
  #include "clients/query.hpp"
  #include "ndns-label.hpp"
  #include "validator.hpp"
  #include "ndns-enum.hpp"
  #include "ndns-tlv.hpp"
  #include "logger.hpp"
  #include "daemon/db-mgr.hpp"
  #include "util/util.hpp"
*/

#include "cpu-temp.hpp"
#include "validator.hpp"

#include <ndn-cxx/encoding/block-helpers.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/data.hpp>
#include <ndn-cxx/util/io.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>
#include <boost/noncopyable.hpp>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include <string>
#include <tuple>

namespace ndn {
namespace ndns {
constexpr int UPDATE_OK = 0;
constexpr int UpdateReturnCode = 160;
constexpr int UpdateReturnMsg = 161;

class NdnsUpdate : noncopyable
{
public:
  NdnsUpdate(const Name& hint, const Name& zone, const shared_ptr<Data>& update, Face& face)
    : m_hint(hint)
    , m_zone(zone)
    , m_interestLifetime(DEFAULT_INTEREST_LIFETIME)
    , m_face(face)
    , m_validator(face)
    , m_update(update)
    , m_hasError(false)
  {
  }

  void
  start()
  {
    std::cout << " ================ "
              << "start to update RR at Zone = " << this->m_zone
              << " new RR is: " << m_update->getName()
              <<" =================== " << std::endl;

    std::cout << "new RR is signed by: "
              << m_update->getSignature().getKeyLocator().getName()
              << std::endl;

    Interest interest = this->makeUpdateInterest();
    std::cout << "[* <- *] send Update: " << interest.getName() << std::endl
              <<"Embedding Data: " << m_update->getName().toUri() << std::endl;
    m_face.expressInterest(interest,
                           bind(&NdnsUpdate::onData, this, _1, _2),
                           bind(&NdnsUpdate::onTimeout, this, _1) //dynamic binding
                           );
  }

  void
  stop()
  {
    m_face.getIoService().stop();
  }

private:
  void
  onData(const Interest& interest, const Data& data)
  {
    std::cout << "get response of Update" << std::endl;
    int ret = -1;
    std::string msg;
    std::tie(ret, msg) = this->parseResponse(data);
    std::cout << "Return Code: " << ret << ", and Update "
              << (ret == UPDATE_OK ? "succeeds" : "fails") << std::endl;
    if (ret != UPDATE_OK)
      m_hasError = true;


    std::cout << "to verify the response" << std::endl;
    m_validator.validate(data,
                         bind(&NdnsUpdate::onDataValidated, this, _1),
                         bind(&NdnsUpdate::onDataValidationFailed, this, _1, _2)
                         );
  }

  std::tuple<int, std::string>
  parseResponse(const Data& data)
  {
    int ret = -1;
    std::string msg;
    Block blk = data.getContent();
    blk.parse();
    Block block = blk.blockFromValue();
    block.parse();
    Block::element_const_iterator val = block.elements_begin();
    for (; val != block.elements_end(); ++val) {
      if (val->type() == UpdateReturnCode) { // the first must be return code
        ret = readNonNegativeInteger(*val);
      }
      else if (val->type() == UpdateReturnMsg) {
        msg =  std::string(reinterpret_cast<const char*>(val->value()), val->value_size());
      }
    }

    return std::make_tuple(ret, msg);
  }

  /**
   * @brief construct a query (interest) which contains the update information
   */
  Interest
  makeUpdateInterest()
  {
    Name name;
    name.append(m_zone)
      .append("NDNS")
      .append(m_update->wireEncode())
      .append("UPDATE");

    Interest interest(name);

    return interest;
  }

private:
  void
  onTimeout(const ndn::Interest& interest)
  {
    std::cout << "Update timeouts" << std::endl;
    m_hasError = true;
    this->stop();
  }

  void
  onDataValidated(const shared_ptr<const Data>& data)
  {
    this->stop();
  }

  void
  onDataValidationFailed(const shared_ptr<const Data>& data, const std::string& str)
  {
    m_hasError = true;
    this->stop();
  }

public:

  void
  setInterestLifetime(const time::milliseconds& interestLifetime)
  {
    m_interestLifetime = interestLifetime;
  }

  const bool
  hasError() const
  {
    return m_hasError;
  }

private:
  Name m_hint;
  Name m_zone;

  time::milliseconds m_interestLifetime;

  Face& m_face;
  Validator m_validator;
  KeyChain m_keyChain;

  shared_ptr<Data> m_update;
  bool m_hasError;
};

} // namespace ndns
} // namespace ndn

int
main(int argc, char* argv[])
{
  using std::string;
  using namespace ndn;
  using namespace ndn::ndns;

  Name hint;
  Name zone;
  int ttl = 4;
  Name rrLabel = "cpu";
  string rrType = "TXT";
  string ndnsTypeStr = "resp";
  Name certName;
  std::vector<string> contents;
  string contentFile;
  shared_ptr<Data> update;

  try {
    namespace po = boost::program_options;
    po::variables_map vm;

    po::options_description generic("Generic Options");
    generic.add_options()("help,h", "print help message");

    po::options_description config("Configuration");
    config.add_options()
      ("hint,H", po::value<Name>(&hint), "forwarding hint")
      ("ttl,T", po::value<int>(&ttl), "TTL of query. default: 4 sec")
      ("rrtype,t", po::value<string>(&rrType), "set request RR Type. default: TXT")
      ("ndnsType,n", po::value<string>(&ndnsTypeStr), "Set the ndnsType of the resource record. "
       "Potential values are [resp|nack|auth|raw]. Default: resp")
      ("cert,c", po::value<Name>(&certName), "set the name of certificate to sign the update")
      ("rrlabel,l", po::value<Name>(&rrLabel), "set request RR Label")
      ;

    po::options_description hidden("Hidden Options");
    hidden.add_options()
      ("zone,z", po::value<Name>(&zone), "zone the record is delegated")
      ;
    po::positional_options_description postion;
    postion.add("zone", 1);

    po::options_description cmdline_options;
    cmdline_options.add(generic).add(config).add(hidden);

    po::options_description config_file_options;
    config_file_options.add(config).add(hidden);

    po::options_description visible("Usage: ndns-update zone rrLabel [-t rrType] [-T TTL] "
                                    "[-H hint] [-n NdnsType] [-c cert] "
                                    "[-f contentFile]|[-o content]\n"
                                    "Allowed options");

    visible.add(generic).add(config);

    po::parsed_options parsed =
      po::command_line_parser(argc, argv).options(cmdline_options).positional(postion).run();

    po::store(parsed, vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << visible << std::endl;
      return 0;
    }

    KeyChain keyChain;
    if (certName.empty()) {
      Name name = Name().append(zone).append(rrLabel);
      // choosing the longest match of the identity who also have default certificate
      for (size_t i = name.size() + 1; i > 0; --i) { // i >=0 will present warnning
        Name tmp = name.getPrefix(i - 1);
        if (keyChain.doesIdentityExist(tmp)) {
          try {
            certName = keyChain.getDefaultCertificateNameForIdentity(tmp);
            break;
          }
          catch (std::exception&) {
            // If it cannot get a default certificate from one identity,
            // just ignore this one try next identity.
            ;
          }
        }
      } // for

      if (certName.empty()) {
        std::cerr << "cannot figure out the certificate automatically. "
                  << "please set it with -c CERT_NAEME" << std::endl;
        return 1;
      }
    }
    else {
      if (!keyChain.doesCertificateExist(certName)) {
        std::cerr << "certificate: " << certName << " does not exist" << std::endl;
        return 1;
      }
    }

    Name name;
    name.append(zone)
      .append("NDNS")
      .append(rrLabel)
      .append(rrType)
      .appendVersion();

    Data data(name);
    CPUTemp sensor;
    sensor.SMCOpen();
    char* key = const_cast<char*>(SMC_KEY_CPU_TEMP);

    std::string temp = std::to_string(sensor.SMCGetTemperature(key));
    Block block = ndn::dataBlock(ndn::tlv::Content, temp.c_str(), temp.size());
    data.setContent(block);
    sensor.SMCClose();

    update = make_shared<Data>(data);
    keyChain.sign(*update, certName);
  }
  catch (const std::exception& ex) {
    std::cerr << "Parameter Error: " << ex.what() << std::endl;
    return 1;
  }

  Face face;
  try {
    NdnsUpdate updater(hint, zone, update, face);
    updater.setInterestLifetime(ndn::time::seconds(ttl));

    updater.start();
    face.processEvents();

    if (updater.hasError())
      return 1;
    else
      return 0;
  }
  catch (const ndn::ValidatorConfig::Error& e) {
    std::cerr << "Fail to create the validator: " << e.what() << std::endl;
    return 1;
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
