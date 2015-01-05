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

#include <ndn-cxx/data.hpp>
#include <ndn-cxx/security/validator-config.hpp>

namespace ndn {
namespace ndns {

std::string Validator::DEFAULT_CONFIG_PATH = "/usr/local/etc/ndns";
std::string Validator::VALIDATOR_CONF_FILE = DEFAULT_CONFIG_PATH + "/" + "validator.conf";

Validator::Validator(Face& face, const std::string& confFile /* = VALIDATOR_CONF_FILE */)
  : ValidatorConfig(face)
{
  try {
    this->load(confFile);
    std::cerr << "Validator loads configuration: " << confFile << std::endl;
  }
  catch (std::exception&) {
    std::string config =
      "rule                                                                       \n"
      "{                                                                          \n"
      "  id \"NDNS Validator\"                                                    \n"
      "  for data                                                                 \n"
      "  checker                                                                  \n"
      "  {                                                                        \n"
      "    type customized                                                        \n"
      "    sig-type rsa-sha256                                                    \n"
      "    key-locator                                                            \n"
      "    {                                                                      \n"
      "      type name                                                            \n"
      "      hyper-relation                                                       \n"
      "      {                                                                    \n"
      "        k-regex ^(<>*)<KEY>(<>*)<><ID-CERT>$                               \n"
      "        k-expand \\\\1\\\\2                                                \n"
      "        h-relation is-prefix-of                                            \n"
      "        p-regex ^(<>*)[<KEY><NDNS>](<>*)<><>$                              \n"
      "        p-expand \\\\1\\\\2                                                \n"
      "      }                                                                    \n"
      "    }                                                                      \n"
      "  }                                                                        \n"
      "}                                                                          \n"
      "                                                                           \n"
      "                                                                           \n"
      "trust-anchor                                                               \n"
      "{                                                                          \n"
      "  type file                                                                \n"
      "  file-name \""
      ;

    config += DEFAULT_CONFIG_PATH + "/" + "anchors/root.cert";

    config +=
      "\"                                                                         \n"
      "}                                                                          \n"
      "                                                                           \n"
      ;

    this->load(config, "embededConf");
    std::cout << "Validator loads embedded configuration with anchors path: anchors/root.cert"
              << std::endl;
  }

}

void
Validator::validate(const Data& data,
                    const OnDataValidated& onValidated,
                    const OnDataValidationFailed& onValidationFailed)
{
  std::cout << "[* ?? *] verify data: " << data.getName() << ". KeyLocator: "
            << data.getSignature().getKeyLocator().getName() << std::endl;
  ValidatorConfig::validate(data,
                            [this, onValidated] (const shared_ptr<const Data>& data) {
                              this->onDataValidated(data);
                              onValidated(data);
                            },
                            [this, onValidationFailed] (const shared_ptr<const Data>& data,
                                                        const std::string& str) {
                              this->onDataValidationFailed(data, str);
                              onValidationFailed(data, str);
                            }
                            );
}

void
Validator::onDataValidated(const shared_ptr<const Data>& data)
{
  std::cout << "[* VV *] pass validation: " << data->getName() << ". KeyLocator = "
            << data->getSignature().getKeyLocator().getName() << std::endl;
}

void
Validator::onDataValidationFailed(const shared_ptr<const Data>& data, const std::string& str)
{
  std::cerr << "[* XX *] fail validation: " << data->getName() << ". due to: " << str
            << ". KeyLocator = " << data->getSignature().getKeyLocator().getName()
            << std::endl;
}

} // namespace ndns
} // namespace ndn
