/**********************************************************************
 * Copyright (c) 2018 Ambr project
 * Distributed under the MIT software license, see the accompanying   *
 * file COPYING or http://www.opensource.org/licenses/mit-license.php.*
 **********************************************************************/
#include "unit.h"
#include <proto/unit.pb.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <crypto/sha256.h>


std::shared_ptr<ambr::core::Unit> ambr::core::Unit::CreateUnitByByte(const std::vector<uint8_t> &buf){
  //MakeShared TODO
  std::shared_ptr<ReceiveUnit> receive_unit = std::shared_ptr<ReceiveUnit>(new ReceiveUnit());
  std::shared_ptr<SendUnit> send_unit = std::shared_ptr<SendUnit>(new SendUnit());
  if(receive_unit->DeSerializeByte(buf)){
    return receive_unit;
  }else if(send_unit->DeSerializeByte(buf)){
    return send_unit;
  }
  return std::shared_ptr<ambr::core::Unit>();
}

ambr::core::Unit::Unit():
  version_(0),
  type_(UnitType::Invalidate),
  public_key_("0"),
  prev_unit_("0"),
  balance_("0"),
  hash_("0"),
  sign_("0"){
}

ambr::core::SendUnit::SendUnit():Unit(){

}


std::string ambr::core::SendUnit::SerializeJson() const{
  ::boost::property_tree::ptree unit_pt;
  unit_pt.put("version", version_);
  unit_pt.put("type", (uint8_t)type_);
  unit_pt.put("public_key", public_key_.encode_to_hex());
  unit_pt.put("prev_unit", prev_unit_.encode_to_hex());
  unit_pt.put("balance", balance_.encode_to_hex());
  unit_pt.put("hash", hash_.encode_to_hex());
  unit_pt.put("sign", sign_.encode_to_hex());
  unit_pt.put("dest", dest_.encode_to_hex());
  ::boost::property_tree::ptree pt;
  pt.add_child("unit", unit_pt);
  ::std::ostringstream stream;
  ::boost::property_tree::write_json(stream, pt);
  return stream.str();
}

bool ambr::core::SendUnit::DeSerializeJson(const std::string& json){
  try{
    ::boost::property_tree::ptree pt;
    std::istringstream stream(json.c_str());
    ::boost::property_tree::read_json(stream, pt);
    version_ = pt.get<uint32_t>("unit.version");
    type_ = (UnitType)pt.get<uint8_t>("unit.type");
    public_key_.decode_from_hex(pt.get<std::string>("unit.public_key"));
    prev_unit_.decode_from_hex(pt.get<std::string>("unit.prev_unit"));
    balance_.decode_from_hex(pt.get<std::string>("unit.balance"));
    hash_.decode_from_hex(pt.get<std::string>("unit.hash"));
    sign_.decode_from_hex(pt.get<std::string>("unit.sign"));
    dest_.decode_from_hex(pt.get<std::string>("unit.dest"));
    return true;
  }catch(::boost::property_tree::json_parser::json_parser_error& error){
    std::cout<<error.message();
    return false;
  }
}

std::vector<uint8_t> ambr::core::SendUnit::SerializeByte( ) const {
  std::vector<uint8_t> buf;
  ::ambr::SendUnit obj;
  obj.set_version_(version_);
  obj.set_type_((ambr::SendUnit::Type)type_);
  obj.set_public_key_(public_key_.bytes().data(), public_key_.bytes().size());
  obj.set_prev_unit_(prev_unit_.bytes().data(),prev_unit_.bytes().size());
  obj.set_balance_(balance_.bytes().data(),balance_.bytes().size());
  obj.set_hash_(hash_.bytes().data(),hash_.bytes().size());
  obj.set_sign_(sign_.bytes().data(),sign_.bytes().size());
  obj.set_dest_(dest_.bytes().data(),dest_.bytes().size());
  size_t len=obj.ByteSize();
  buf.resize(obj.ByteSize());
  obj.SerializeToArray(buf.data(), len);
  return buf;
}

bool ambr::core::SendUnit::DeSerializeByte(const std::vector<uint8_t> &buf,size_t* used_size){
  ::ambr::SendUnit obj;
  //if(obj.ParseFromArray(buf.data(),buf.size())){
  google::protobuf::io::CodedInputStream stream(buf.data(),buf.size());
  if(obj.ParseFromCodedStream(&stream)){
    version_= (uint32_t)obj.version_();
    type_=((ambr::core::UnitType)obj.type_());
    public_key_.set_bytes(obj.public_key_().data(),obj.public_key_().size());
    prev_unit_.set_bytes(obj.prev_unit_().data(),obj.prev_unit_().size());
    balance_.set_bytes(obj.balance_().data(),obj.balance_().size());
    hash_.set_bytes(obj.hash_().data(),obj.hash_().size());
    sign_.set_bytes(obj.sign_().data(),obj.sign_().size());
    dest_.set_bytes(obj.dest_().data(),obj.dest_().size());
    return true;
  }
  return false;
}

ambr::core::UnitHash ambr::core::SendUnit::CalcHash() const {
  crypto::SHA256OneByOneHasher hasher;
  hasher.init();
  hasher.process(version_);
  hasher.process(type_);
  hasher.process(public_key_);
  hasher.process(prev_unit_);
  hasher.process(balance_);
  hasher.process(dest_);
  hasher.finish();
  UnitHash::ArrayType array;
  hasher.get_hash_bytes(array.begin(), array.end());
  return UnitHash(array);
}

void ambr::core::SendUnit::CalcHashAndFill(){
  hash_ = CalcHash();
}

bool ambr::core::SendUnit::SignatureAndFill(const ambr::core::PrivateKey &key){
  sign_ = GetSignByPrivateKey(hash_.bytes().data(), hash_.bytes().size(), key);
  return true;
}

bool ambr::core::SendUnit::Validate(std::string *err) const{
  //check version
  if(version_ != 0x00000001){
    if(err){
      *err = "error version";
    }
    return false;
  }
  //check hash
  if(hash_ != CalcHash()){
    if(err){
      *err = "error hash";
    }
    return false;
  }
  if(!ambr::core::SignIsValidate(hash_.bytes().data(), hash_.bytes().size(), public_key_, sign_)){
    *err = "error signature";
    return false;
  }
  return true;
}

ambr::core::ReceiveUnit::ReceiveUnit():Unit(){

}

std::string ambr::core::ReceiveUnit::SerializeJson() const{
  ::boost::property_tree::ptree unit_pt;
  unit_pt.put("version", version_);
  unit_pt.put("type", (uint8_t)type_);
  unit_pt.put("public_key", public_key_.encode_to_hex());
  unit_pt.put("prev_unit", prev_unit_.encode_to_hex());
  unit_pt.put("balance", balance_.encode_to_hex());
  unit_pt.put("hash", hash_.encode_to_hex());
  unit_pt.put("sign", sign_.encode_to_hex());
  unit_pt.put("from", from_.encode_to_hex());
  ::boost::property_tree::ptree pt;
  pt.add_child("unit", unit_pt);
  ::std::ostringstream stream;
  ::boost::property_tree::write_json(stream, pt);
  return stream.str();
}

bool ambr::core::ReceiveUnit::DeSerializeJson(const std::string &json){
  try{
    ::boost::property_tree::ptree pt;
    std::istringstream stream(json.c_str());
    ::boost::property_tree::read_json(stream, pt);
    version_ = pt.get<uint32_t>("unit.version");
    type_ = (UnitType)pt.get<uint8_t>("unit.type");
    public_key_.decode_from_hex(pt.get<std::string>("unit.public_key"));
    prev_unit_.decode_from_hex(pt.get<std::string>("unit.prev_unit"));
    balance_.decode_from_hex(pt.get<std::string>("unit.balance"));
    hash_.decode_from_hex(pt.get<std::string>("unit.hash"));
    sign_.decode_from_hex(pt.get<std::string>("unit.sign"));
    from_.decode_from_hex(pt.get<std::string>("unit.from"));
    return true;
  }catch(::boost::property_tree::json_parser::json_parser_error& error){
    std::cout<<error.message();
    return false;
  }
}

std::vector<uint8_t> ambr::core::ReceiveUnit::SerializeByte( ) const {
  std::vector<uint8_t> buf;
  ::ambr::ReceiveUnit obj;
  obj.set_version_(version_);
  obj.set_type_((ambr::ReceiveUnit::Type)type_);
  obj.set_public_key_(public_key_.bytes().data(), public_key_.bytes().size());
  obj.set_prev_unit_(prev_unit_.bytes().data(),prev_unit_.bytes().size());
  obj.set_balance_(balance_.bytes().data(),balance_.bytes().size());
  obj.set_hash_(hash_.bytes().data(),hash_.bytes().size());
  obj.set_sign_(sign_.bytes().data(),sign_.bytes().size());
  obj.set_from_(from_.bytes().data(),from_.bytes().size());
  size_t len=obj.ByteSize();
  buf.resize(obj.ByteSize());
  obj.SerializeToArray(buf.data(), len);
  return buf;
}

bool ambr::core::ReceiveUnit::DeSerializeByte(const std::vector<uint8_t> &buf,size_t* used_size){
  ::ambr::ReceiveUnit obj;

  google::protobuf::io::CodedInputStream stream(buf.data(),buf.size());
  if(obj.ParseFromCodedStream(&stream)){
    version_= (uint32_t)obj.version_();
    type_=((ambr::core::UnitType)obj.type_());
    public_key_.set_bytes(obj.public_key_().data(),obj.public_key_().size());
    prev_unit_.set_bytes(obj.prev_unit_().data(),obj.prev_unit_().size());
    balance_.set_bytes(obj.balance_().data(),obj.balance_().size());
    hash_.set_bytes(obj.hash_().data(),obj.hash_().size());
    sign_.set_bytes(obj.sign_().data(),obj.sign_().size());
    from_.set_bytes(obj.from_().data(),obj.from_().size());
    return true;
  }
  return false;
}

ambr::core::UnitHash ambr::core::ReceiveUnit::CalcHash() const {
  crypto::SHA256OneByOneHasher hasher;
  hasher.init();
  hasher.process(version_);
  hasher.process(type_);
  hasher.process(public_key_);
  hasher.process(prev_unit_);
  hasher.process(balance_);
  hasher.process(from_);
  hasher.finish();
  UnitHash::ArrayType array;
  hasher.get_hash_bytes(array.begin(), array.end());
  return UnitHash(array);
}

void ambr::core::ReceiveUnit::CalcHashAndFill(){
  hash_ = CalcHash();
}

bool ambr::core::ReceiveUnit::SignatureAndFill(const ambr::core::PrivateKey &key){
  //TODO:
  Signature::ArrayType array_sign;
  PrivateKey::ArrayType array_key;
  for(size_t i = 0; i< array_key.size(); i++){
    array_sign[i]=array_key[i];
  }
  sign_.set_bytes(array_sign);
  return true;
}

bool ambr::core::ReceiveUnit::Validate(std::string *err) const{
  //check version
  if(version_ != 0x00000001){
    if(err){
      *err = "error version";
    }
    return false;
  }
  //check hash
  if(hash_ != CalcHash()){
    if(err){
      *err = "error hash";
    }
    return false;
  }
  //check signature
  //TODO:
  return true;
}

ambr::core::VoteUnit::VoteUnit():Unit(){

}

std::string ambr::core::VoteUnit::SerializeJson() const{
  ::boost::property_tree::ptree unit_pt;
  unit_pt.put("version", version_);
  unit_pt.put("type", (uint8_t)type_);
  unit_pt.put("public_key", public_key_.encode_to_hex());
  unit_pt.put("prev_unit", prev_unit_.encode_to_hex());
  unit_pt.put("balance", balance_.encode_to_hex());
  unit_pt.put("hash", hash_.encode_to_hex());
  unit_pt.put("sign", sign_.encode_to_hex());
  unit_pt.put("validator_unit_hash", validator_unit_hash_.encode_to_hex());
  unit_pt.put("accept", accept_);
  ::boost::property_tree::ptree pt;
  pt.add_child("unit", unit_pt);
  ::std::ostringstream stream;
  ::boost::property_tree::write_json(stream, pt);
  return stream.str();
}

bool ambr::core::VoteUnit::DeSerializeJson(const std::string& json){
  try{
    ::boost::property_tree::ptree pt;
    std::istringstream stream(json.c_str());
    ::boost::property_tree::read_json(stream, pt);
    version_ = pt.get<uint32_t>("unit.version");
    type_ = (UnitType)pt.get<uint8_t>("unit.type");
    public_key_.decode_from_hex(pt.get<std::string>("unit.public_key"));
    prev_unit_.decode_from_hex(pt.get<std::string>("unit.prev_unit"));
    balance_.decode_from_hex(pt.get<std::string>("unit.balance"));
    hash_.decode_from_hex(pt.get<std::string>("unit.hash"));
    sign_.decode_from_hex(pt.get<std::string>("unit.sign"));
    validator_unit_hash_.decode_from_hex(pt.get<std::string>("unit.validator_unit_hash"));
    accept_ = (uint8_t)pt.get<uint8_t>("unit.accept");
    return true;
  }catch(::boost::property_tree::json_parser::json_parser_error& error){
    std::cout<<error.message();
    return false;
  }
}

std::vector<uint8_t> ambr::core::VoteUnit::SerializeByte( ) const {
  std::vector<uint8_t> buf;
  ::ambr::VoteUnit obj;
  obj.set_version_(version_);
  obj.set_type_((ambr::VoteUnit::Type)type_);
  obj.set_public_key_(public_key_.bytes().data(), public_key_.bytes().size());
  obj.set_prev_unit_(prev_unit_.bytes().data(),prev_unit_.bytes().size());
  obj.set_balance_(balance_.bytes().data(),balance_.bytes().size());
  obj.set_hash_(hash_.bytes().data(),hash_.bytes().size());
  obj.set_sign_(sign_.bytes().data(),sign_.bytes().size());
  obj.set_validator_unit_hash_(validator_unit_hash_.bytes().data(),validator_unit_hash_.bytes().size());
  obj.set_accept_((uint8_t)accept_);
  size_t len=obj.ByteSize();
  buf.resize(obj.ByteSize());
  obj.SerializeToArray(buf.data(), len);
  return buf;
}

bool ambr::core::VoteUnit::DeSerializeByte(const std::vector<uint8_t> &buf,size_t* used_size){
  ::ambr::VoteUnit obj;

  google::protobuf::io::CodedInputStream stream(buf.data(),buf.size());
  if(obj.ParseFromCodedStream(&stream)){
    version_= (uint32_t)obj.version_();
    type_=((ambr::core::UnitType)obj.type_());
    public_key_.set_bytes(obj.public_key_().data(),obj.public_key_().size());
    prev_unit_.set_bytes(obj.prev_unit_().data(),obj.prev_unit_().size());
    balance_.set_bytes(obj.balance_().data(),obj.balance_().size());
    hash_.set_bytes(obj.hash_().data(),obj.hash_().size());
    sign_.set_bytes(obj.sign_().data(),obj.sign_().size());
    validator_unit_hash_.set_bytes(obj.validator_unit_hash_().data(),obj.validator_unit_hash_().size());
    accept_=(uint8_t)obj.accept_();
    return true;
  }
  return false;
}

ambr::core::UnitHash ambr::core::VoteUnit::CalcHash() const {
  crypto::SHA256OneByOneHasher hasher;
  hasher.init();
  hasher.process(version_);
  hasher.process(type_);
  hasher.process(public_key_);
  hasher.process(prev_unit_);
  hasher.process(balance_);
  hasher.process(validator_unit_hash_);
  hasher.process(accept_);
  hasher.finish();
  UnitHash::ArrayType array;
  hasher.get_hash_bytes(array.begin(), array.end());
  return UnitHash(array);
}

void ambr::core::VoteUnit::CalcHashAndFill(){
  hash_ = CalcHash();
}

bool ambr::core::VoteUnit::SignatureAndFill(const ambr::core::PrivateKey &key){
  sign_ = GetSignByPrivateKey(hash_.bytes().data(), hash_.bytes().size(), key);
  return true;
}

bool ambr::core::VoteUnit::Validate(std::string *err) const{
  //check version
  if(version_ != 0x00000001){
    if(err){
      *err = "error version";
    }
    return false;
  }
  //check hash
  if(hash_ != CalcHash()){
    if(err){
      *err = "error hash";
    }
    return false;
  }
  if(!ambr::core::SignIsValidate(hash_.bytes().data(), hash_.bytes().size(), public_key_, sign_)){
    *err = "error signature";
    return false;
  }
  return true;
}


ambr::core::ValidatorUnit::ValidatorUnit():Unit(){

}

std::string ambr::core::ValidatorUnit::SerializeJson() const{
  ::boost::property_tree::ptree unit_pt;
  unit_pt.put("version", version_);
  unit_pt.put("type", (uint8_t)type_);
  unit_pt.put("public_key", public_key_.encode_to_hex());
  unit_pt.put("prev_unit", prev_unit_.encode_to_hex());
  unit_pt.put("balance", balance_.encode_to_hex());
  unit_pt.put("hash", hash_.encode_to_hex());
  unit_pt.put("sign", sign_.encode_to_hex());
  ::boost::property_tree::ptree pt_child;
  pt_child.clear();
  for(UnitHash hash:check_list_){
    ::boost::property_tree::ptree tmp;
    tmp.put("", hash.encode_to_hex());
    pt_child.push_back(std::make_pair("", tmp));
  }
  unit_pt.add_child("check_list", pt_child);

  pt_child.clear();
  for(UnitHash hash:vote_hash_list_){
    ::boost::property_tree::ptree tmp;
    tmp.put("", hash.encode_to_hex());
    pt_child.push_back(std::make_pair("", tmp));
  }
  unit_pt.add_child("vote_hash_list", pt_child);



  pt_child.clear();
  for(VoteUnit unit:vote_list_){
    ::boost::property_tree::ptree tmp;
    ::boost::property_tree::ptree vote_tree;
    //std::istringstream istream(unit.SerializeJson());
    //::boost::property_tree::read_json(istream, vote_tree);
    tmp.put("", unit.SerializeJson());
    pt_child.push_back(std::make_pair("", tmp));
  }
  unit_pt.add_child("vote_list", pt_child);
  unit_pt.put("percent", percent_);
  unit_pt.put("time_stamp", time_stamp_);
  unit_pt.put("nonce", nonce_);
  ::boost::property_tree::ptree pt;
  pt.add_child("unit", unit_pt);
  ::std::ostringstream stream;
  ::boost::property_tree::write_json(stream, pt);
  return stream.str();
}

bool ambr::core::ValidatorUnit::DeSerializeJson(const std::string& json){
  try{
    ::boost::property_tree::ptree pt;
    std::istringstream stream(json.c_str());
    ::boost::property_tree::read_json(stream, pt);
    version_ = pt.get<uint32_t>("unit.version");
    type_ = (UnitType)pt.get<uint8_t>("unit.type");
    public_key_.decode_from_hex(pt.get<std::string>("unit.public_key"));
    prev_unit_.decode_from_hex(pt.get<std::string>("unit.prev_unit"));
    balance_.decode_from_hex(pt.get<std::string>("unit.balance"));
    hash_.decode_from_hex(pt.get<std::string>("unit.hash"));
    sign_.decode_from_hex(pt.get<std::string>("unit.sign"));
    ::boost::property_tree::ptree pt_array;
    pt_array = pt.get_child("unit.check_list");
    for(auto child: pt_array){
      UnitHash hash;
      hash.decode_from_hex(child.second.data());
      check_list_.push_back(hash);
    }

    pt_array = pt.get_child("unit.vote_hash_list");
    for(auto child: pt_array){
      UnitHash hash;
      hash.decode_from_hex(child.second.data());
      vote_hash_list_.push_back(hash);
    }

    pt_array = pt.get_child("unit.vote_list");
    for(auto child: pt_array){
      VoteUnit vote_unit;
      vote_unit.DeSerializeJson(child.second.data());
      vote_list_.push_back(vote_unit);
    }
    percent_ = pt.get<uint32_t>("unit.percent");
    time_stamp_ = pt.get<time_t>("unit.time_stamp");
    nonce_ = pt.get<uint64_t>("unit.nonce");
    return true;
  }catch(::boost::property_tree::json_parser::json_parser_error& error){
    std::cout<<error.message();
    return false;
  }
}

std::vector<uint8_t> ambr::core::ValidatorUnit::SerializeByte( ) const {
  std::vector<uint8_t> buf;
  ::ambr::ValidatorUnit obj;
  obj.set_version_(version_);
  obj.set_type_((ambr::ValidatorUnit::Type)type_);
  obj.set_public_key_(public_key_.bytes().data(), public_key_.bytes().size());
  obj.set_prev_unit_(prev_unit_.bytes().data(),prev_unit_.bytes().size());
  obj.set_balance_(balance_.bytes().data(),balance_.bytes().size());
  obj.set_hash_(hash_.bytes().data(),hash_.bytes().size());
  obj.set_sign_(sign_.bytes().data(),sign_.bytes().size());

  for(UnitHash hash: check_list_){
   std::string check_hash;
   check_hash.resize(sizeof(hash));
   memcpy((void *)check_hash.data(), &hash, sizeof(hash));
    *obj.mutable_check_list_()->Add() = check_hash;
  }

  for(UnitHash hash: vote_hash_list_){
    std::string vote_hash;
    vote_hash.resize(sizeof(hash));
    memcpy((void *)vote_hash.data(), &hash, sizeof(hash));
    *obj.mutable_vote_hash_list_()->Add()=vote_hash;
  }

   obj.set_percent_(percent_);

  for(VoteUnit unit: vote_list_){
    ::ambr::VoteUnit vote_unit_proto;
    vote_unit_proto.set_version_(unit.version());
    vote_unit_proto.set_type_((ambr::VoteUnit::Type)unit.type());
    vote_unit_proto.set_public_key_(unit.public_key().bytes().data(), unit.public_key().bytes().size());
    vote_unit_proto.set_prev_unit_(unit.prev_unit().bytes().data(), unit.prev_unit().bytes().size());
    vote_unit_proto.set_balance_(unit.balance().bytes().data(), unit.balance().bytes().size());
    vote_unit_proto.set_hash_(unit.hash().bytes().data(), unit.hash().bytes().size());
    vote_unit_proto.set_sign_(unit.sign().bytes().data(), unit.sign().bytes().size());
    vote_unit_proto.set_validator_unit_hash_(unit.ValidatorUnitHash().bytes().data(), unit.ValidatorUnitHash().bytes().size());
    vote_unit_proto.set_accept_((uint32_t)unit.accept());

    *obj.mutable_vote_list_()->Add() = vote_unit_proto;
  }
    obj.set_nonce_(nonce_);
    obj.set_time_stamp_(time_stamp_);

    size_t len=obj.ByteSize();
    buf.resize(obj.ByteSize());
    obj.SerializeToArray(buf.data(), len);
  return buf;
}

bool ambr::core::ValidatorUnit::DeSerializeByte(const std::vector<uint8_t> &buf,size_t* used_size){
  ::ambr::ValidatorUnit obj;

  check_list_.clear();
  vote_hash_list_.clear();
  vote_list_.clear();
  google::protobuf::io::CodedInputStream stream(buf.data(),buf.size());
  if(obj.ParseFromCodedStream(&stream)){
    version_= (uint32_t)obj.version_();
    type_=((ambr::core::UnitType)obj.type_());
    public_key_.set_bytes(obj.public_key_().data(),obj.public_key_().size());
    prev_unit_.set_bytes(obj.prev_unit_().data(),obj.prev_unit_().size());
    balance_.set_bytes(obj.balance_().data(),obj.balance_().size());
    hash_.set_bytes(obj.hash_().data(),obj.hash_().size());
    sign_.set_bytes(obj.sign_().data(),obj.sign_().size());

    for(int i = 0; i < obj.check_list_().size(); i++){
      UnitHash hash;
      hash.set_bytes(obj.check_list_(i).data(), obj.check_list_(i).size());
      check_list_.push_back(hash);
    }

    for(int i=0; i<obj.vote_hash_list_().size(); i++){
      UnitHash hash;
      hash.set_bytes(obj.vote_hash_list_(i).data(), obj.vote_hash_list_(i).size());
      vote_hash_list_.push_back(hash);
    }

    percent_=(uint32_t)obj.percent_();

    for(int i=0; i<obj.vote_list_().size(); i++){
      ambr::core::VoteUnit proto_unit;
      proto_unit.set_version((uint32_t)obj.vote_list_(i).version_());
      proto_unit.set_type((ambr::core::UnitType)obj.vote_list_(i).type_());

      PublicKey publickey;
      publickey.set_bytes(obj.vote_list_(i).public_key_().data(), obj.vote_list_(i).public_key_().size());
      proto_unit.set_public_key(publickey);

      UnitHash prevunit;
      prevunit.set_bytes(obj.vote_list_(i).prev_unit_().data(), obj.vote_list_(i).prev_unit_().size());
      proto_unit.set_prev_unit(prevunit);

      Amount amount;
      amount.set_bytes(obj.vote_list_(i).balance_().data(), obj.vote_list_(i).balance_().size());
      proto_unit.set_balance(amount);

      UnitHash hash;
      hash.set_bytes(obj.vote_list_(i).hash_().data(), obj.vote_list_(i).hash_().size());
      proto_unit.set_hash(hash);

      Signature sign;
      sign.set_bytes(obj.vote_list_(i).sign_().data(), obj.vote_list_(i).sign_().size());
      proto_unit.set_sign(sign);

      UnitHash validator_unit_hash;
      validator_unit_hash.set_bytes(obj.vote_list_(i).validator_unit_hash_().data(), obj.vote_list_(i).validator_unit_hash_().size());
      proto_unit.set_validator_unit_hash(validator_unit_hash);

      bool accept;
      accept=obj.vote_list_(i).accept_()==0?0:1;
      proto_unit.set_accept(accept);

      vote_list_.push_back(proto_unit);
    }

    nonce_=(uint64_t)obj.nonce_();
    time_stamp_=(uint64_t)obj.time_stamp_();
    return true;
  }
  return false;
}

ambr::core::UnitHash ambr::core::ValidatorUnit::CalcHash() const {
  crypto::SHA256OneByOneHasher hasher;
  hasher.init();
  hasher.process(version_);
  hasher.process(type_);
  hasher.process(public_key_);
  hasher.process(prev_unit_);
  hasher.process(balance_);
  for(UnitHash hash:check_list_){
    hasher.process(hash);
  }
  for(UnitHash hash:vote_hash_list_){
    hasher.process(hash);
  }
  for(VoteUnit unit:vote_list_){
    hasher.process(unit.hash());
  }
  hasher.process(percent_);
  hasher.process(time_stamp_);
  hasher.process(nonce_);
  hasher.finish();
  UnitHash::ArrayType array;
  hasher.get_hash_bytes(array.begin(), array.end());
  return UnitHash(array);
}

void ambr::core::ValidatorUnit::CalcHashAndFill(){
  hash_ = CalcHash();
}

bool ambr::core::ValidatorUnit::SignatureAndFill(const ambr::core::PrivateKey &key){
  sign_ = GetSignByPrivateKey(hash_.bytes().data(), hash_.bytes().size(), key);
  return true;
}

bool ambr::core::ValidatorUnit::Validate(std::string *err) const{
  //check version
  if(version_ != 0x00000001){
    if(err){
      *err = "error version";
    }
    return false;
  }
  //check hash
  if(hash_ != CalcHash()){
    if(err){
      *err = "error hash";
    }
    return false;
  }
  if(!ambr::core::SignIsValidate(hash_.bytes().data(), hash_.bytes().size(), public_key_, sign_)){
    *err = "error signature";
    return false;
  }
  return true;
}



ambr::core::EnterValidateSetUint::EnterValidateSetUint():Unit(){

}

std::string ambr::core::EnterValidateSetUint::SerializeJson() const{
  ::boost::property_tree::ptree unit_pt;
  unit_pt.put("version", version_);
  unit_pt.put("type", (uint8_t)type_);
  unit_pt.put("public_key", public_key_.encode_to_hex());
  unit_pt.put("prev_unit", prev_unit_.encode_to_hex());
  unit_pt.put("balance", balance_.encode_to_hex());
  unit_pt.put("hash", hash_.encode_to_hex());
  unit_pt.put("sign", sign_.encode_to_hex());
  ::boost::property_tree::ptree pt;
  pt.add_child("unit", unit_pt);
  ::std::ostringstream stream;
  ::boost::property_tree::write_json(stream, pt);
  return stream.str();
}

bool ambr::core::EnterValidateSetUint::DeSerializeJson(const std::string& json){
  try{
    ::boost::property_tree::ptree pt;
    std::istringstream stream(json.c_str());
    ::boost::property_tree::read_json(stream, pt);
    version_ = pt.get<uint32_t>("unit.version");
    type_ = (UnitType)pt.get<uint8_t>("unit.type");
    public_key_.decode_from_hex(pt.get<std::string>("unit.public_key"));
    prev_unit_.decode_from_hex(pt.get<std::string>("unit.prev_unit"));
    balance_.decode_from_hex(pt.get<std::string>("unit.balance"));
    hash_.decode_from_hex(pt.get<std::string>("unit.hash"));
    sign_.decode_from_hex(pt.get<std::string>("unit.sign"));
    return true;
  }catch(::boost::property_tree::json_parser::json_parser_error& error){
    std::cout<<error.message();
    return false;
  }
}

std::vector<uint8_t> ambr::core::EnterValidateSetUint::SerializeByte( ) const {
  std::vector<uint8_t> buf;
  ::ambr::EnterValidateSetUint obj;
  obj.set_version_(version_);
  obj.set_type_((ambr::EnterValidateSetUint::Type)type_);
  obj.set_public_key_(public_key_.bytes().data(), public_key_.bytes().size());
  obj.set_prev_unit_(prev_unit_.bytes().data(),prev_unit_.bytes().size());
  obj.set_balance_(balance_.bytes().data(),balance_.bytes().size());
  obj.set_hash_(hash_.bytes().data(),hash_.bytes().size());
  obj.set_sign_(sign_.bytes().data(),sign_.bytes().size());
  size_t len=obj.ByteSize();
  buf.resize(obj.ByteSize());
  obj.SerializeToArray(buf.data(), len);
  return buf;
}

bool ambr::core::EnterValidateSetUint::DeSerializeByte(const std::vector<uint8_t> &buf,size_t* used_size){
  ::ambr::EnterValidateSetUint obj;
  google::protobuf::io::CodedInputStream stream(buf.data(),buf.size());
  if(obj.ParseFromCodedStream(&stream)){
    version_= (uint32_t)obj.version_();
    type_=((ambr::core::UnitType)obj.type_());
    public_key_.set_bytes(obj.public_key_().data(),obj.public_key_().size());
    prev_unit_.set_bytes(obj.prev_unit_().data(),obj.prev_unit_().size());
    balance_.set_bytes(obj.balance_().data(),obj.balance_().size());
    hash_.set_bytes(obj.hash_().data(),obj.hash_().size());
    sign_.set_bytes(obj.sign_().data(),obj.sign_().size());
    return true;
  }
  return false;
}

ambr::core::UnitHash ambr::core::EnterValidateSetUint::CalcHash() const {
  crypto::SHA256OneByOneHasher hasher;
  hasher.init();
  hasher.process(version_);
  hasher.process(type_);
  hasher.process(public_key_);
  hasher.process(prev_unit_);
  hasher.process(balance_);
  hasher.finish();
  UnitHash::ArrayType array;
  hasher.get_hash_bytes(array.begin(), array.end());
  return UnitHash(array);
}

void ambr::core::EnterValidateSetUint::CalcHashAndFill(){
  hash_ = CalcHash();
}

bool ambr::core::EnterValidateSetUint::SignatureAndFill(const ambr::core::PrivateKey &key){
  sign_ = GetSignByPrivateKey(hash_.bytes().data(), hash_.bytes().size(), key);
  return true;
}

bool ambr::core::EnterValidateSetUint::Validate(std::string *err) const{
  //check version
  if(version_ != 0x00000001){
    if(err){
      *err = "error version";
    }
    return false;
  }
  //check hash
  if(hash_ != CalcHash()){
    if(err){
      *err = "error hash";
    }
    return false;
  }
  if(!ambr::core::SignIsValidate(hash_.bytes().data(), hash_.bytes().size(), public_key_, sign_)){
    *err = "error signature";
    return false;
  }
  return true;
}


ambr::core::LeaveValidateSetUint::LeaveValidateSetUint():Unit(){

}

std::string ambr::core::LeaveValidateSetUint::SerializeJson() const{
  ::boost::property_tree::ptree unit_pt;
  unit_pt.put("version", version_);
  unit_pt.put("type", (uint8_t)type_);
  unit_pt.put("public_key", public_key_.encode_to_hex());
  unit_pt.put("prev_unit", prev_unit_.encode_to_hex());
  unit_pt.put("balance", balance_.encode_to_hex());
  unit_pt.put("hash", hash_.encode_to_hex());
  unit_pt.put("sign", sign_.encode_to_hex());
  ::boost::property_tree::ptree pt;
  pt.add_child("unit", unit_pt);
  ::std::ostringstream stream;
  ::boost::property_tree::write_json(stream, pt);
  return stream.str();
}

bool ambr::core::LeaveValidateSetUint::DeSerializeJson(const std::string& json){
  try{
    ::boost::property_tree::ptree pt;
    std::istringstream stream(json.c_str());
    ::boost::property_tree::read_json(stream, pt);
    version_ = pt.get<uint32_t>("unit.version");
    type_ = (UnitType)pt.get<uint8_t>("unit.type");
    public_key_.decode_from_hex(pt.get<std::string>("unit.public_key"));
    prev_unit_.decode_from_hex(pt.get<std::string>("unit.prev_unit"));
    balance_.decode_from_hex(pt.get<std::string>("unit.balance"));
    hash_.decode_from_hex(pt.get<std::string>("unit.hash"));
    sign_.decode_from_hex(pt.get<std::string>("unit.sign"));
    return true;
  }catch(::boost::property_tree::json_parser::json_parser_error& error){
    std::cout<<error.message();
    return false;
  }
}

std::vector<uint8_t> ambr::core::LeaveValidateSetUint::SerializeByte( ) const {
  std::vector<uint8_t> buf;
  ::ambr::LeaveValidateSetUint obj;
  obj.set_version_(version_);
  obj.set_type_((ambr::LeaveValidateSetUint::Type)type_);
  obj.set_public_key_(public_key_.bytes().data(), public_key_.bytes().size());
  obj.set_prev_unit_(prev_unit_.bytes().data(),prev_unit_.bytes().size());
  obj.set_balance_(balance_.bytes().data(),balance_.bytes().size());
  obj.set_hash_(hash_.bytes().data(),hash_.bytes().size());
  obj.set_sign_(sign_.bytes().data(),sign_.bytes().size());
  size_t len=obj.ByteSize();
  buf.resize(obj.ByteSize());
  obj.SerializeToArray(buf.data(), len);
  return buf;
}

bool ambr::core::LeaveValidateSetUint::DeSerializeByte(const std::vector<uint8_t> &buf,size_t* used_size){
  ::ambr::LeaveValidateSetUint obj;
  google::protobuf::io::CodedInputStream stream(buf.data(),buf.size());
  if(obj.ParseFromCodedStream(&stream)){
    version_= (uint32_t)obj.version_();
    type_=((ambr::core::UnitType)obj.type_());
    public_key_.set_bytes(obj.public_key_().data(),obj.public_key_().size());
    prev_unit_.set_bytes(obj.prev_unit_().data(),obj.prev_unit_().size());
    balance_.set_bytes(obj.balance_().data(),obj.balance_().size());
    hash_.set_bytes(obj.hash_().data(),obj.hash_().size());
    sign_.set_bytes(obj.sign_().data(),obj.sign_().size());
    return true;
  }
  return false;
}

ambr::core::UnitHash ambr::core::LeaveValidateSetUint::CalcHash() const {
  crypto::SHA256OneByOneHasher hasher;
  hasher.init();
  hasher.process(version_);
  hasher.process(type_);
  hasher.process(public_key_);
  hasher.process(prev_unit_);
  hasher.process(balance_);
  hasher.finish();
  UnitHash::ArrayType array;
  hasher.get_hash_bytes(array.begin(), array.end());
  return UnitHash(array);
}

void ambr::core::LeaveValidateSetUint::CalcHashAndFill(){
  hash_ = CalcHash();
}

bool ambr::core::LeaveValidateSetUint::SignatureAndFill(const ambr::core::PrivateKey &key){
  sign_ = GetSignByPrivateKey(hash_.bytes().data(), hash_.bytes().size(), key);
  return true;
}

bool ambr::core::LeaveValidateSetUint::Validate(std::string *err) const{
  //check version
  if(version_ != 0x00000001){
    if(err){
      *err = "error version";
    }
    return false;
  }
  //check hash
  if(hash_ != CalcHash()){
    if(err){
      *err = "error hash";
    }
    return false;
  }
  if(!ambr::core::SignIsValidate(hash_.bytes().data(), hash_.bytes().size(), public_key_, sign_)){
    *err = "error signature";
    return false;
  }
  return true;
}


