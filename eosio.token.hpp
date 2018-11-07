/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>

#include <string>

namespace eosiosystem
{
class system_contract;
}

namespace eosio
{

using std::string;

CONTRACT token : public eosio::contract
{
    public:
      using contract::contract;
      //keep key words 

      token(name self) : contract(self) {}

      void create(name issuer,
                  asset maximum_supply);

      void issue(name to, asset quantity, string memo);

      void retire(asset quantity, string memo);

      void transfer(name from,
                    name to,
                    asset quantity,
                    string memo);

      void close(name owner, symbol symbol);

      inline asset get_supply(symbol sym) const;

      inline asset get_balance(name owner, symbol sym) const;

    private:
      struct [[eosio::table]] account
     {
            asset balance;

            uint64_t primary_key() const { return balance.symbol.name(); }
      };

      struct currency_stats
      {
            asset supply;
            asset max_supply;
            name issuer;

            uint64_t primary_key() const { return supply.symbol.name(); }
      };

      typedef eosio::multi_index<name("accounts"), account> accounts;
      typedef eosio::multi_index<name("stat"), currency_stats> stats;

      void sub_balance(name owner, asset value);
      void add_balance(name owner, asset value, name ram_payer);

    public:
      struct transfer_args
      {
            name from;
            name to;
            asset quantity;
            string memo;
      };
};

asset token::get_supply(symbol sym) const
{
      stats statstable(_self, sym);
      const auto &st = statstable.get(sym);
      return st.supply;
}

asset token::get_balance(name owner, symbol sym) const
{
      accounts accountstable(_self, owner);
      const auto &ac = accountstable.get(sym);
      return ac.balance;
}

} // namespace eosio
