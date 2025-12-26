/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef STIRLITZ_H
#define STIRLITZ_H

#include <filesystem>
#include <gcrypt.h>
#include <memory>
#include <string>
#include <vector>

/*!
 * \mainpage Stirlitz
 *
 * Stirlitz is a simple library for encrypting and decrypting text messages and
 * files of any format. It also provides interface for generating public keys
 * based on Ed25519 algorithm. Stirilitz uses libgcrypt under hood. Only AES256
 * algorithm is available for encryption now.
 *
 * To use Stirlitz in your project include stirlitz CMake package in your
 * project
 * \code {.unparsed}
 * find_package(stirlitz REQUIRED)
 * target_link_libraries(my_target stirlitz::stirlitz)
 * \endcode
 */

/*!
 * \brief The Stirlitz class
 *
 * Base class of Stirlitz library.
 */
class Stirlitz
{
public:
  /*!
   * \brief Stirlitz constructor
   */
  Stirlitz();

  /*!
   * \brief Calculates hash summ for given string.
   *
   * \param data String hash summ to be calculated for.
   * \param algo Hash algorithm. See libgcrypt <A
   * HREF="https://www.gnupg.org/documentation/manuals/gcrypt/Available-hash-algorithms.html#Available-hash-algorithms">
   * documentation</A> for details.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \return Vector containing hash summ.
   */
  std::vector<unsigned char>
  hashString(const std::string &data, const int &algo);

  /*!
   * \brief Converts given data to hexadecimal format.
   * \param data Data to be converted. Can be std::string or
   * std::vector<unsigned char> on user choice.
   * \return std::string containing data in hexadecimal format (two hexadecimal
   * digits for each byte).
   */
  template <typename T>
  std::string
  toHex(const T &data);

  /*!
   * \brief Converts hexadecimal string to \"normal\" bytes.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param hex Data to be converted.
   * \return std::string containing result of conversion (one byte for each two
   * hexadecimal digits).
   */
  std::string
  fromHex(const std::string &hex);

  /*!
   * \brief Generates Ed25519 key pair.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \return Smart pointer to generated key pair <A
   * HREF="https://www.gnupg.org/documentation/manuals/gcrypt/S_002dexpressions.html#S_002dexpressions">S-expression</A>
   */
  std::shared_ptr<gcry_sexp>
  generateKeyPair();

  /*!
   * \brief Encrypts given data.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param username User name.
   * \param password Password.
   * \param data Data to be encrypted.
   * \return std::string containing encrypted data.
   */
  std::string
  encryptData(const std::string &username, const std::string &password,
              const std::string &data);

  /*!
   * \brief Decrypts given data.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param username User name.
   * \param password Password.
   * \param data Data to be decrypted.
   * \return std::string containing decrypted data.
   */
  std::string
  decryptData(const std::string &username, const std::string &password,
              const std::string &data);

  /*!
   * \brief Encrypts given file.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param source_file Path to file to be encrypted.
   * \param result Path to file result of encryption to be saved to.
   * \param username User name.
   * \param password Password.
   */
  void
  encryptFile(const std::filesystem::path &source_file,
              const std::filesystem::path &result, const std::string &username,
              const std::string &password);

  /*!
   * \brief Decrypts given file.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param source_file Path to file to be decrypted.
   * \param result Path to file result of decryption to be saved to.
   * \param username User name.
   * \param password Password.
   */
  void
  decryptFile(const std::filesystem::path &source_file,
              const std::filesystem::path &result, const std::string &username,
              const std::string &password);

  /*!
   * \brief Converts S-expression object to string.
   * \param exp Smart pointer to S-expression.
   * \return String representation of S-expression object.
   */
  std::string
  sexpToString(std::shared_ptr<gcry_sexp> exp);

  /*!
   * \brief Converst S-expression string representation to S-expression object.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param data String representation of S-expression.
   * \return Smart pointer to S-expression object.
   */
  std::shared_ptr<gcry_sexp>
  sexpFromString(const std::string &data);

  /*!
   * \brief Creates hexadecimal representation of public key.
   * \param key Smart pointer to public key S-expression or to key pair
   * S-expression.
   * \return Hexadecimal representation of public key (2 digits for each byte).
   */
  std::string
  getPublicKeyString(std::shared_ptr<gcry_sexp> key);

  /*!
   * \brief Generates public key S-expression object from raw data.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param key std::string containing public key raw data (NOT in hexadecimal
   * format).
   * \return Smart pointer to public key S-expression.
   */
  std::shared_ptr<gcry_sexp>
  generatePublicKeyExp(const std::string &key);

  /*!
   * \brief Generates user name and password from given Ed25519 keys.
   *
   * This method can be used to generate user name and password from your
   * Ed25519 key pair and your opponent Ed25519 public key (variant for
   * encryption).
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param own_key_pair Smart pointer to your key pair S-expression object.
   * \param opponent_key Smart pointer to opponent public key S-expression
   * object.
   * \return std::tuple containing user name and password. First element of
   * tuple is user name, second element - is password.
   */
  std::tuple<std::string, std::string>
  genUsernamePasswordEncryption(std::shared_ptr<gcry_sexp> own_key_pair,
                                std::shared_ptr<gcry_sexp> opponent_key);

  /*!
   * \brief Generates user name and password from given Ed25519 keys.
   *
   * This method can be used to generate user name and password from your
   * Ed25519 key pair and your opponent Ed25519 public key (variant for
   * decryption).
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param own_key_pair Smart pointer to your key pair S-expression object.
   * \param opponent_key Smart pointer to opponent public key S-expression
   * object.
   * \return std::tuple containing user name and password. First element of
   * tuple is user name, second element - is password.
   */
  std::tuple<std::string, std::string>
  genUsernamePasswordDecryption(std::shared_ptr<gcry_sexp> own_key_pair,
                                std::shared_ptr<gcry_sexp> opponent_key);

private:
  void
  printGcryptError(const gcry_error_t &err, const std::string &prefix);
};

#endif // STIRLITZ_H
