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

#include <Stirlitz.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <sstream>
#include <stdexcept>

#ifndef __ANDROID__
#include <iostream>
#else
#define APPNAME "Stirlitz"
#include <android/log.h>
#endif

Stirlitz::Stirlitz()
{
  gcry_error_t err = gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P, 0);
  if(!err)
    {
      err = gcry_control(GCRYCTL_ENABLE_M_GUARD);
      if(err != 0)
        {
          try
            {
              printGcryptError(err, "strilitz: libgcrypt memory guard error");
            }
          catch(std::exception &er)
            {
#ifndef __ANDROID__
              std::cout << er.what() << std::endl;
#else
              __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "%s",
                                  er.what());
#endif
            }
        }
      const char *ver = gcry_check_version(NULL);
      if(ver)
        {
          err = gcry_control(GCRYCTL_INIT_SECMEM, 32768);
          if(err != 0)
            {
              printGcryptError(
                  err, "strilitz: libgcrypt secmem initialization error");
            }
          err = gcry_control(GCRYCTL_AUTO_EXPAND_SECMEM, 32768);
          if(err != 0)
            {
              printGcryptError(
                  err, "strilitz: libgcrypt secmem auto expand set error");
            }
          err = gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
          if(err == 0)
            {
#ifndef __ANDROID__
              std::cout << "strilitz: libgcrypt has been "
                           "initialized, version "
                        << ver << std::endl;
#else
              __android_log_print(
                  ANDROID_LOG_VERBOSE, APPNAME,
                  "strilitz: libgcrypt has been initialized, version %s", ver);
#endif
            }
          else
            {
              printGcryptError(
                  err, "strilitz: libgcrypt has not been initialized!");
            }
        }
    }
}

std::vector<unsigned char>
Stirlitz::hashString(const std::string &data, const int &algo)
{
  std::vector<unsigned char> result;

  gcry_md_hd_t hd_t;
  gcry_error_t err = gcry_md_open(&hd_t, algo, GCRY_MD_FLAG_SECURE);
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::hashString");
    }
  std::unique_ptr<gcry_md_handle, std::function<void(gcry_md_handle *)>> hd(
      hd_t,
      [](gcry_md_handle *hd)
        {
          gcry_md_close(hd);
        });

  gcry_md_write(hd.get(), data.c_str(), data.size());

  unsigned char *hsh = gcry_md_read(hd.get(), algo);

  unsigned int len = gcry_md_get_algo_dlen(algo);
  result.reserve(len);

  for(unsigned int i = 0; i < len; i++)
    {
      result.push_back(hsh[i]);
    }

  return result;
}

std::string
Stirlitz::fromHex(const std::string &hex)
{
  if(hex.size() % 2 != 0)
    {
      throw std::runtime_error(
          "Stirlitz::fromHex: incompatible size of hex value");
    }
  std::string result;
  result.resize(hex.size() / 2);

  std::locale loc("C");
  size_t count = 0;
  for(size_t i = 0; i < hex.size(); i += 2)
    {
      std::string tmp(hex.begin() + i, hex.begin() + i + 2);
      std::stringstream strm;
      strm.imbue(loc);
      strm.str(tmp);
      int val;
      strm >> std::hex >> val;
      uint8_t val8 = static_cast<uint8_t>(val);
      std::memcpy(&result[count], &val8, sizeof(val8));
      count++;
    }

  return result;
}

template std::string
Stirlitz::toHex(const std::vector<unsigned char> &data);
template std::string
Stirlitz::toHex(const std::string &data);
template <typename T>
std::string
Stirlitz::toHex(const T &val)
{
  std::string result;
  result.resize(val.size() * 2);
  size_t count = 0;
  std::locale loc("C");
  std::for_each(val.begin(), val.end(),
                [&result, &count, loc](auto &el)
                  {
                    uint8_t val8;
                    std::memcpy(&val8, &el, sizeof(el));
                    std::stringstream strm;
                    strm.imbue(loc);
                    strm << std::hex << static_cast<int>(val8);
                    if(val8 <= 15)
                      {
                        result[count] = '0';
                        result[count + 1] = strm.str()[0];
                      }
                    else
                      {
                        result[count] = strm.str()[0];
                        result[count + 1] = strm.str()[1];
                      }
                    count += 2;
                  });
  return result;
}

std::shared_ptr<gcry_sexp>
Stirlitz::generateKeyPair()
{
  std::shared_ptr<gcry_sexp> result;

  std::string param = "(genkey"
                      "  (ecdh"
                      "    (curve \"Ed25519\")"
                      "    (flags eddsa)"
                      "  )"
                      ")";
  gcry_sexp_t exp_ptr;
  gcry_error_t err = gcry_sexp_new(&exp_ptr, param.c_str(), param.size(), 1);
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::generateKeyPair");
    }

  std::unique_ptr<gcry_sexp, std::function<void(gcry_sexp *)>> param_ptr(
      exp_ptr,
      [](gcry_sexp *exp)
        {
          gcry_sexp_release(exp);
        });

  err = gcry_pk_genkey(&exp_ptr, param_ptr.get());
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::generateKeyPair");
    }

  result = std::shared_ptr<gcry_sexp>(exp_ptr,
                                      [](gcry_sexp *exp)
                                        {
                                          gcry_sexp_release(exp);
                                        });

  return result;
}

std::string
Stirlitz::encryptData(const std::string &username, const std::string &password,
                      const std::string &data)
{
  std::string result;

  gcry_cipher_hd_t handle;

  gcry_error_t err
      = gcry_cipher_open(&handle, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC,
                         GCRY_CIPHER_CBC_CTS | GCRY_CIPHER_SECURE);
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::encryptData");
    }

  std::unique_ptr<gcry_cipher_handle,
                  std::function<void(gcry_cipher_handle *)>>
      hd(handle,
         [](gcry_cipher_handle *hd)
           {
             gcry_cipher_close(hd);
           });

  std::vector<unsigned char> hash = hashString(username, GCRY_MD_BLAKE2S_128);

  err = gcry_cipher_setiv(hd.get(), hash.data(), hash.size());
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::encryptData");
    }

  hash = hashString(password, GCRY_MD_BLAKE2S_256);

  err = gcry_cipher_setkey(hd.get(), hash.data(), hash.size());
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::encryptData");
    }

  std::vector<unsigned char> in;
  in.reserve(data.size() + 1);
  in.resize(data.size());
  std::memcpy(in.data(), data.c_str(), data.size());
  size_t block_sz = gcry_cipher_get_algo_blklen(GCRY_CIPHER_AES256);
  size_t sz = in.size();
  unsigned char extra = 0;
  if(sz < block_sz)
    {
      in.resize(block_sz);
      extra = static_cast<unsigned char>(block_sz - sz);
      gcry_randomize(in.data() + sz, block_sz - sz, GCRY_WEAK_RANDOM);
    }
  in.push_back(extra);

  result.resize(in.size());

  err = gcry_cipher_encrypt(hd.get(),
                            reinterpret_cast<unsigned char *>(result.data()),
                            result.size(), in.data(), in.size());
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::encryptData");
    }

  return result;
}

std::string
Stirlitz::decryptData(const std::string &username, const std::string &password,
                      const std::string &data)
{
  std::string result;
  if(data.empty())
    {
      return result;
    }

  gcry_cipher_hd_t handle;

  gcry_error_t err
      = gcry_cipher_open(&handle, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC,
                         GCRY_CIPHER_CBC_CTS | GCRY_CIPHER_SECURE);
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::decryptData");
    }

  std::unique_ptr<gcry_cipher_handle,
                  std::function<void(gcry_cipher_handle *)>>
      hd(handle,
         [](gcry_cipher_handle *hd)
           {
             gcry_cipher_close(hd);
           });

  std::vector<unsigned char> hash = hashString(username, GCRY_MD_BLAKE2S_128);

  err = gcry_cipher_setiv(hd.get(), hash.data(), hash.size());
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::decryptData");
    }

  hash = hashString(password, GCRY_MD_BLAKE2S_256);

  err = gcry_cipher_setkey(hd.get(), hash.data(), hash.size());
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::decryptData");
    }

  result.resize(data.size());

  err = gcry_cipher_decrypt(
      hd.get(), reinterpret_cast<unsigned char *>(result.data()),
      result.size(), reinterpret_cast<const unsigned char *>(data.c_str()),
      data.size());
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::decryptData");
    }

  unsigned char extra = static_cast<unsigned char>(*result.rbegin());
  if(result.size() > 0)
    {
      result.pop_back();
    }
  while(extra > 0 && result.size() > 0)
    {
      result.pop_back();
      extra--;
    }
  result.shrink_to_fit();

  return result;
}

void
Stirlitz::encryptFile(const std::filesystem::path &source_file,
                      const std::filesystem::path &result,
                      const std::string &username, const std::string &password)
{
  gcry_cipher_hd_t handle;

  gcry_error_t err
      = gcry_cipher_open(&handle, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC,
                         GCRY_CIPHER_CBC_CTS | GCRY_CIPHER_SECURE);
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::encryptFile:");
    }

  std::unique_ptr<gcry_cipher_handle,
                  std::function<void(gcry_cipher_handle *)>>
      hd(handle,
         [](gcry_cipher_handle *hd)
           {
             gcry_cipher_close(hd);
           });

  std::vector<unsigned char> hash = hashString(password, GCRY_MD_BLAKE2S_256);

  err = gcry_cipher_setkey(hd.get(), hash.data(), hash.size());
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::encryptFile:");
    }

  std::fstream f_source;
  f_source.open(source_file, std::ios_base::in | std::ios_base::binary);
  if(!f_source.is_open())
    {
      throw std::runtime_error(
          "Stirlitz::encryptFile: cannot open source file");
    }

  std::filesystem::create_directories(result.parent_path());
  std::filesystem::remove_all(result);
  std::fstream f_result;
  f_result.open(result, std::ios_base::out | std::ios_base::binary);
  if(!f_result.is_open())
    {
      f_source.close();
      throw std::runtime_error(
          "Stirlitz::encryptFile: cannot write to resulting file");
    }

  size_t fsz;
  f_source.seekg(0, std::ios_base::end);
  fsz = f_source.tellg();
  f_source.seekg(0, std::ios_base::beg);

  if(fsz == 0)
    {
      f_source.close();
      throw std::runtime_error("Stirlitz::encryptFile: incorrect file");
    }

  size_t read_b = 0;
  size_t buf_sz = 1024;
  std::vector<unsigned char> buf;
  buf.reserve(buf_sz);
  std::vector<unsigned char> result_buf;
  result_buf.reserve(buf_sz);

  size_t block_sz = gcry_cipher_get_algo_blklen(GCRY_CIPHER_AES256);

  size_t dif;

  char extra_b = 0;
  size_t sz;
  hash = hashString(username, GCRY_MD_BLAKE2S_128);
  while(read_b < fsz)
    {
      err = gcry_cipher_setiv(hd.get(), hash.data(), hash.size());
      if(err != 0)
        {
          f_source.close();
          f_result.close();
          std::filesystem::remove_all(result);
          printGcryptError(err, "Stirlitz::encryptFile gcry_cipher_setiv:");
        }
      buf.clear();
      dif = fsz - read_b;
      if(buf_sz < dif)
        {
          buf.resize(buf_sz);
        }
      else
        {
          buf.resize(dif);
        }
      f_source.read(reinterpret_cast<char *>(buf.data()), buf.size());
      read_b += buf.size();

      sz = buf.size();
      if(sz < block_sz)
        {
          size_t extra = block_sz - sz;
          buf.resize(block_sz);
          gcry_randomize(buf.data() + sz, extra, GCRY_WEAK_RANDOM);
          extra_b = static_cast<char>(extra);
        }

      result_buf.clear();
      result_buf.resize(buf.size());

      err = gcry_cipher_encrypt(hd.get(), result_buf.data(), result_buf.size(),
                                buf.data(), buf.size());
      if(err != 0)
        {
          f_source.close();
          f_result.close();
          std::filesystem::remove_all(result);
          printGcryptError(err, "Stirlitz::encryptFile:");
        }
      f_result.write(reinterpret_cast<char *>(result_buf.data()),
                     result_buf.size());

      err = gcry_cipher_reset(hd.get());
      if(err != 0)
        {
          f_source.close();
          f_result.close();
          std::filesystem::remove_all(result);
          printGcryptError(err, "Stirlitz::encryptFile gcry_cipher_reset:");
        }
    }

  f_result.write(&extra_b, sizeof(extra_b));

  f_source.close();
  f_result.close();
}

void
Stirlitz::decryptFile(const std::filesystem::path &source_file,
                      const std::filesystem::path &result,
                      const std::string &username, const std::string &password)
{
  gcry_cipher_hd_t handle;

  gcry_error_t err
      = gcry_cipher_open(&handle, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC,
                         GCRY_CIPHER_CBC_CTS | GCRY_CIPHER_SECURE);
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::decryptFile:");
    }

  std::unique_ptr<gcry_cipher_handle,
                  std::function<void(gcry_cipher_handle *)>>
      hd(handle,
         [](gcry_cipher_handle *hd)
           {
             gcry_cipher_close(hd);
           });

  std::vector<unsigned char> hash = hashString(password, GCRY_MD_BLAKE2S_256);

  err = gcry_cipher_setkey(hd.get(), hash.data(), hash.size());
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::decryptFile:");
    }

  std::fstream f_source;
  f_source.open(source_file, std::ios_base::in | std::ios_base::binary);
  if(!f_source.is_open())
    {
      throw std::runtime_error(
          "Stirlitz::decryptFile: cannot open source file");
    }

  std::filesystem::create_directories(result.parent_path());
  std::filesystem::remove_all(result);
  std::fstream f_result;
  f_result.open(result, std::ios_base::out | std::ios_base::binary);
  if(!f_result.is_open())
    {
      f_source.close();
      throw std::runtime_error(
          "Stirlitz::decryptFile: cannot write to resulting file");
    }

  size_t fsz;
  f_source.seekg(0, std::ios_base::end);
  fsz = f_source.tellg();
  f_source.seekg(0, std::ios_base::beg);

  if(fsz == 0)
    {
      f_source.close();
      throw std::runtime_error("Stirlitz::decryptFile: incorrect file(1)");
    }

  size_t read_b = 0;
  size_t buf_sz = 1024;
  std::vector<unsigned char> buf;
  buf.reserve(buf_sz);
  std::vector<unsigned char> result_buf;
  result_buf.reserve(buf_sz);

  size_t block_sz = gcry_cipher_get_algo_blklen(GCRY_CIPHER_AES256);

  size_t dif;

  size_t sz;
  hash = hashString(username, GCRY_MD_BLAKE2S_128);
  bool last = false;
  while(read_b < fsz - 1)
    {
      err = gcry_cipher_setiv(hd.get(), hash.data(), hash.size());
      if(err != 0)
        {
          printGcryptError(err, "Stirlitz::decryptFile gcry_cipher_setiv:");
        }
      buf.clear();
      dif = fsz - read_b;
      if(buf_sz < dif)
        {
          buf.resize(buf_sz);
        }
      else
        {
          buf.resize(dif - 1);
          last = true;
        }
      f_source.read(reinterpret_cast<char *>(buf.data()), buf.size());
      read_b += buf.size();

      sz = buf.size();
      if(sz < block_sz)
        {
          f_source.close();
          f_result.close();
          std::filesystem::remove_all(result);
          throw std::runtime_error("Stirlitz::decryptFile: incorrect file(2)");
        }

      result_buf.clear();
      result_buf.resize(buf.size());

      err = gcry_cipher_decrypt(hd.get(), result_buf.data(), result_buf.size(),
                                buf.data(), buf.size());
      if(err != 0)
        {
          f_source.close();
          f_result.close();
          std::filesystem::remove_all(result);
          printGcryptError(err, "Stirlitz::decryptFile:");
        }
      if(last)
        {
          char val;
          f_source.read(&val, sizeof(val));
          read_b += sizeof(val);
          sz = static_cast<size_t>(val);
          if(sz <= result_buf.size())
            {
              f_result.write(reinterpret_cast<char *>(result_buf.data()),
                             result_buf.size() - sz);
            }
          else
            {
              f_source.close();
              f_result.close();
              std::filesystem::remove_all(result);
              throw std::runtime_error(
                  "Stirlitz::decryptFile: incorrect file(3)");
            }
        }
      else
        {
          f_result.write(reinterpret_cast<char *>(result_buf.data()),
                         result_buf.size());
        }
      err = gcry_cipher_reset(hd.get());
      if(err != 0)
        {
          f_source.close();
          f_result.close();
          std::filesystem::remove_all(result);
          printGcryptError(err, "Stirlitz::decryptFile gcry_cipher_reset:");
        }
    }

  f_source.close();
  f_result.close();
}

std::string
Stirlitz::sexpToString(std::shared_ptr<gcry_sexp> exp)
{
  std::string result;

  if(exp)
    {
      size_t sz
          = gcry_sexp_sprint(exp.get(), GCRYSEXP_FMT_DEFAULT, nullptr, 0);
      result.resize(sz);
      gcry_sexp_sprint(exp.get(), GCRYSEXP_FMT_DEFAULT, result.data(), sz);
    }

  return result;
}

std::shared_ptr<gcry_sexp>
Stirlitz::sexpFromString(const std::string &data)
{
  std::shared_ptr<gcry_sexp> result;

  gcry_sexp_t exp;
  gcry_error_t err = gcry_sexp_new(&exp, data.c_str(), data.size(), 1);
  if(err != 0)
    {
      printGcryptError(err, "Stirlitz::sexpFromString");
    }

  result = std::shared_ptr<gcry_sexp>(exp,
                                      [](gcry_sexp *exp)
                                        {
                                          gcry_sexp_release(exp);
                                        });

  return result;
}

std::string
Stirlitz::getPublicKeyString(std::shared_ptr<gcry_sexp> key)
{
  std::string result;

  std::string find_str = "public-key";
  gcry_sexp_t exp
      = gcry_sexp_find_token(key.get(), find_str.c_str(), find_str.size());
  if(exp == nullptr)
    {
      return result;
    }

  std::shared_ptr<gcry_sexp> pk(exp,
                                [](gcry_sexp *exp)
                                  {
                                    gcry_sexp_release(exp);
                                  });

  std::string pk_str = sexpToString(pk);
  find_str = "1:q";
  std::string::size_type n = pk_str.find(find_str);
  if(n == std::string::npos)
    {
      return result;
    }
  n += find_str.size();

  find_str = ":";
  std::string::size_type n2 = pk_str.find(find_str, n);
  if(n2 == std::string::npos)
    {
      return result;
    }

  std::string num_str;
  std::copy(pk_str.begin() + n, pk_str.begin() + n2,
            std::back_inserter(num_str));
  std::stringstream strm;
  strm.imbue(std::locale("C"));
  strm.str(num_str);
  size_t sz;
  strm >> sz;
  n2 += find_str.size();
  if(n2 + sz < pk_str.size())
    {
      std::string val;
      std::copy(pk_str.begin() + n2, pk_str.begin() + n2 + sz,
                std::back_inserter(val));
      result = toHex(val);
    }

  return result;
}

std::shared_ptr<gcry_sexp>
Stirlitz::generatePublicKeyExp(const std::string &key)
{
  if(key.size() != 32)
    {
      throw std::runtime_error(
          "Stirlitz::generatePublicKeyExp: incorrect key size");
    }

  std::string sexp_format
      = "(public-key (ecc (curve Ed25519)(flags eddsa)(q %b)))";

  gcry_sexp_t exp;
  size_t erroffset = 0;
  gcry_error_t err = gcry_sexp_build(&exp, &erroffset, sexp_format.c_str(),
                                     static_cast<int>(key.size()),
                                     const_cast<char *>(key.data()));
  if(err != 0)
    {
      std::string prefix("Stirlitz::generatePublicKeyExp error: ");
      std::stringstream strm;
      strm.imbue(std::locale("C"));
      strm << erroffset;
      prefix += " byte number " + strm.str();
      printGcryptError(err, prefix);
    }

  std::shared_ptr<gcry_sexp> result(exp,
                                    [](gcry_sexp *exp)
                                      {
                                        gcry_sexp_release(exp);
                                      });

  return result;
}

std::tuple<std::string, std::string>
Stirlitz::genUsernamePasswordEncryption(
    std::shared_ptr<gcry_sexp> own_key_pair,
    std::shared_ptr<gcry_sexp> opponent_key)
{
  std::tuple<std::string, std::string> result;

  gcry_sexp_t exp;
  gcry_error_t err
      = gcry_pk_encrypt(&exp, own_key_pair.get(), own_key_pair.get());
  if(err)
    {
      printGcryptError(err, "Stirlitz::genUsernamePasswordEncryption:");
    }

  std::unique_ptr<gcry_sexp, std::function<void(gcry_sexp *)>> shared(
      exp,
      [](gcry_sexp *exp)
        {
          gcry_sexp_release(exp);
        });

  exp = gcry_sexp_find_token(shared.get(), "ecdh", 4);
  if(exp == nullptr)
    {
      throw std::runtime_error(
          "Stirlitz::genUsernamePasswordEncryption: cannot find token(1)");
    }

  shared = std::unique_ptr<gcry_sexp, std::function<void(gcry_sexp *)>>(
      exp,
      [](gcry_sexp *exp)
        {
          gcry_sexp_release(exp);
        });

  exp = gcry_sexp_find_token(shared.get(), "s", 1);
  if(exp == nullptr)
    {
      throw std::runtime_error(
          "Stirlitz::genUsernamePasswordEncryption: cannot find token(2)");
    }

  shared = std::unique_ptr<gcry_sexp, std::function<void(gcry_sexp *)>>(
      exp,
      [](gcry_sexp *exp)
        {
          gcry_sexp_release(exp);
        });

  size_t sz;
  const char *s_val = gcry_sexp_nth_data(exp, 1, &sz);
  if(s_val)
    {
      std::get<1>(result).resize(sz);
      char *data = std::get<1>(result).data();
      for(size_t i = 0; i < sz; i++)
        {
          data[i] = s_val[i];
        }
    }
  else
    {
      throw std::runtime_error(
          "Stirlitz::genUsernamePasswordEncryption: incorrect value(1)");
    }

  err = gcry_pk_encrypt(&exp, own_key_pair.get(), opponent_key.get());
  if(err)
    {
      printGcryptError(err, "Stirlitz::genUsernamePasswordEncryption:");
    }

  shared = std::unique_ptr<gcry_sexp, std::function<void(gcry_sexp *)>>(
      exp,
      [](gcry_sexp *exp)
        {
          gcry_sexp_release(exp);
        });

  exp = gcry_sexp_find_token(shared.get(), "ecdh", 4);
  if(exp == nullptr)
    {
      throw std::runtime_error(
          "Stirlitz::genUsernamePasswordEncryption: cannot find token(3)");
    }

  shared = std::unique_ptr<gcry_sexp, std::function<void(gcry_sexp *)>>(
      exp,
      [](gcry_sexp *exp)
        {
          gcry_sexp_release(exp);
        });

  exp = gcry_sexp_find_token(shared.get(), "s", 1);
  if(exp == nullptr)
    {
      throw std::runtime_error(
          "Stirlitz::genUsernamePasswordEncryption: cannot find token(4)");
    }

  shared = std::unique_ptr<gcry_sexp, std::function<void(gcry_sexp *)>>(
      exp,
      [](gcry_sexp *exp)
        {
          gcry_sexp_release(exp);
        });

  s_val = gcry_sexp_nth_data(exp, 1, &sz);
  if(s_val)
    {
      std::get<0>(result).resize(sz);
      char *data = std::get<0>(result).data();
      for(size_t i = 0; i < sz; i++)
        {
          data[i] = s_val[i];
        }
    }
  else
    {
      throw std::runtime_error(
          "Stirlitz::genUsernamePasswordEncryption: incorrect value(2)");
    }

  return result;
}

std::tuple<std::string, std::string>
Stirlitz::genUsernamePasswordDecryption(
    std::shared_ptr<gcry_sexp> own_key_pair,
    std::shared_ptr<gcry_sexp> opponent_key)
{
  std::tuple<std::string, std::string> result;

  gcry_sexp_t exp;
  gcry_error_t err
      = gcry_pk_encrypt(&exp, own_key_pair.get(), own_key_pair.get());
  if(err)
    {
      printGcryptError(err, "Stirlitz::genUsernamePasswordDecryption:");
    }

  std::unique_ptr<gcry_sexp, std::function<void(gcry_sexp *)>> shared(
      exp,
      [](gcry_sexp *exp)
        {
          gcry_sexp_release(exp);
        });

  exp = gcry_sexp_find_token(shared.get(), "ecdh", 4);
  if(exp == nullptr)
    {
      throw std::runtime_error(
          "Stirlitz::genUsernamePasswordDecryption: cannot find token(1)");
    }

  shared = std::unique_ptr<gcry_sexp, std::function<void(gcry_sexp *)>>(
      exp,
      [](gcry_sexp *exp)
        {
          gcry_sexp_release(exp);
        });

  exp = gcry_sexp_find_token(shared.get(), "s", 1);
  if(exp == nullptr)
    {
      throw std::runtime_error(
          "Stirlitz::genUsernamePasswordDecryption: cannot find token(2)");
    }

  shared = std::unique_ptr<gcry_sexp, std::function<void(gcry_sexp *)>>(
      exp,
      [](gcry_sexp *exp)
        {
          gcry_sexp_release(exp);
        });

  size_t sz;
  const char *s_val = gcry_sexp_nth_data(exp, 1, &sz);
  if(s_val)
    {
      std::get<0>(result).resize(sz);
      char *data = std::get<0>(result).data();
      for(size_t i = 0; i < sz; i++)
        {
          data[i] = s_val[i];
        }
    }
  else
    {
      throw std::runtime_error(
          "Stirlitz::genUsernamePasswordDecryption: incorrect value(1)");
    }

  err = gcry_pk_encrypt(&exp, own_key_pair.get(), opponent_key.get());
  if(err)
    {
      printGcryptError(err, "Stirlitz::genUsernamePasswordDecryption:");
    }

  shared = std::unique_ptr<gcry_sexp, std::function<void(gcry_sexp *)>>(
      exp,
      [](gcry_sexp *exp)
        {
          gcry_sexp_release(exp);
        });

  exp = gcry_sexp_find_token(shared.get(), "ecdh", 4);
  if(exp == nullptr)
    {
      throw std::runtime_error(
          "Stirlitz::genUsernamePasswordDecryption: cannot find token(3)");
    }

  shared = std::unique_ptr<gcry_sexp, std::function<void(gcry_sexp *)>>(
      exp,
      [](gcry_sexp *exp)
        {
          gcry_sexp_release(exp);
        });

  exp = gcry_sexp_find_token(shared.get(), "s", 1);
  if(exp == nullptr)
    {
      throw std::runtime_error(
          "Stirlitz::genUsernamePasswordDecryption: cannot find token(4)");
    }

  shared = std::unique_ptr<gcry_sexp, std::function<void(gcry_sexp *)>>(
      exp,
      [](gcry_sexp *exp)
        {
          gcry_sexp_release(exp);
        });

  s_val = gcry_sexp_nth_data(exp, 1, &sz);
  if(s_val)
    {
      std::get<1>(result).resize(sz);
      char *data = std::get<1>(result).data();
      for(size_t i = 0; i < sz; i++)
        {
          data[i] = s_val[i];
        }
    }
  else
    {
      throw std::runtime_error(
          "Stirlitz::genUsernamePasswordDecryption: incorrect value(1)");
    }

  return result;
}

void
Stirlitz::printGcryptError(const gcry_error_t &err, const std::string &prefix)
{
  std::string errstr;
  errstr.resize(1024);
  gpg_strerror_r(err, errstr.data(), errstr.size());
  errstr.erase(std::remove_if(errstr.begin(), errstr.end(),
                              [](const char &el)
                                {
                                  if(el)
                                    {
                                      return false;
                                    }
                                  else
                                    {
                                      return true;
                                    }
                                }),
               errstr.end());
  std::stringstream strm;
  strm.imbue(std::locale("C"));
  strm << err;
  if(!errstr.empty())
    {
      errstr = prefix + " " + strm.str() + " (" + errstr + ")";
    }
  else
    {
      errstr = prefix + " " + strm.str();
    }
  throw std::runtime_error(errstr);
}
