#include <algorithm>
#include <eosiolib/transaction.hpp>
#include <eosiolib/name.hpp>
//token 
#include "eosio.token.hpp"
//types
#include "types.hpp"
//new 
#include <eosiolib/eosio.hpp>

using namespace eosio;

//eosio 
CONTRACT eosbocai2222 : public eosio::contract
{
    //change name 
  public:
    using contract::contract
    eosbocai2222(name receiver,name code,datastream<const char*> ds)
        : contract(receiver,code,ds),
          _bets(receiver, receiver),
          _users(receiver, receiver),
          _fund_pool(receiver, receiver),
          _global(receiver, receiver){};

//set action
    void transfer(const account_name &from, const account_name &to, const asset &quantity, const string &memo);

   ACTION void reveal(const st_bet &bet);

   ACTION void init();

//bets
  private:
    tb_bets _bets;
    tb_uesrs _users;
    tb_fund_pool _fund_pool;
    tb_global _global;


//usual 
    void parse_memo(string memo,
                    uint8_t *roll_under,
                    name *referrer)
    {
        // remove space
        memo.erase(std::remove_if(memo.begin(),
                                  memo.end(),
                                  [](unsigned char x) { return std::isspace(x); }),
                   memo.end());

        size_t sep_count = std::count(memo.begin(), memo.end(), '-');
        eosio_assert(sep_count == 2, "invalid memo");

        size_t pos;
        string container;
        pos = sub2sep(memo, &container, '-', 0, true);
        pos = sub2sep(memo, &container, '-', ++pos, true);
        eosio_assert(!container.empty(), "no roll under");
        *roll_under = stoi(container);
        container = memo.substr(++pos);
        if (container.empty())
        {
            *referrer = PRIZEPOOL;
        }
        else
        {
            //remove string_to_name   ??  name("eosio.token"_n)
            *referrer = name("eosio.token"_n);
        }
    }

//asset
    asset compute_referrer_reward(const st_bet &bet) { return bet.amount * 151 / 100000; } //10% for ref
    asset compute_dev_reward(const st_bet &bet) { return bet.amount * 302 / 100000; }      // 20% for dev
    asset compute_pool_reward(const st_bet &bet) { return bet.amount * 1057 / 100000; }    // 70% for pool

    uint64_t next_id()
    {
        //singleton 
        st_global global = _global.get_or_default();
        global.current_id += 1;
        _global.set(global, receiver);
        return global.current_id;
    }

    string referrer_memo(const st_bet &bet)
    {
        string memo = "bet id:";
        string id = uint64_string(bet.id);
        memo.append(id);
        memo.append(" player: ");
        string player = name{bet.player}.to_string();
        memo.append(player);
        memo.append(" referral reward! eosdice.vip");
        return memo;
    }

    string winner_memo(const st_bet &bet)
    {
        string memo = "bet id:";
        string id = uint64_string(bet.id);
        memo.append(id);
        memo.append(" player: ");
        string player = name{bet.player}.to_string();
        memo.append(player);
        memo.append(" winner! eosdice.vip");
        return memo;
    }

    void assert_quantity(const asset &quantity)
    {
        eosio_assert(quantity.symbol == EOS_SYMBOL, "only EOS token allowed");
        eosio_assert(quantity.is_valid(), "quantity invalid");
        //精度10000 0.5 
        eosio_assert(quantity.amount >= 5000, "transfer quantity must be greater than 0.5");
    }

    void assert_roll_under(const uint8_t &roll_under, const asset &quantity)
    {
        eosio_assert(roll_under >= 2 && roll_under <= 96,
                     "roll under overflow, must be greater than 2 and less than 96");
        eosio_assert(
            max_payout(roll_under, quantity) <= max_bonus(),
            "offered overflow, expected earning is greater than the maximum bonus");
    }

    void unlock(const asset &amount)
    {
        st_fund_pool pool = get_fund_pool();
        pool.locked -= amount;
        eosio_assert(pool.locked.amount >= 0, "fund unlock error");
        _fund_pool.set(pool, receiver);
    }

    void lock(const asset &amount)
    {
        st_fund_pool pool = get_fund_pool();
        pool.locked += amount;
        _fund_pool.set(pool, receiver);
    }

    asset compute_payout(const uint8_t &roll_under, const asset &offer)
    {
        return min(max_payout(roll_under, offer), max_bonus());
    }
    asset max_payout(const uint8_t &roll_under, const asset &offer)
    {
        const double ODDS = 98.5 / ((double)roll_under - 1.0);
        return asset(ODDS * offer.amount, offer.symbol);
    }


    asset max_bonus() { return available_balance() / 100; }

//balance 
    asset available_balance()
    {
        auto token = eosio::token(name("eosio.token"_n));
        const asset balance =
            token.get_balance(receiver, eosio::symbol(EOS_SYMBOL).name());
        const asset locked = get_fund_pool().locked;
        const asset available = balance - locked;
        eosio_assert(available.amount >= 0, "fund pool overdraw");
        return available;
    }

    st_fund_pool get_fund_pool()
    {
        st_fund_pool fund_pool{.locked = asset(0, EOS_SYMBOL)};
        return _fund_pool.get_or_create(receiver, fund_pool);
    }

    template <typename... Args>
    void send_defer_action(Args &&... args)
    {
        transaction trx;
        trx.actions.emplace_back(std::forward<Args>(args)...);
        trx.delay_sec = 1;
        trx.send(next_id(), receiver, false);
    }

    uint64_t getDiceSupply()
    {
        auto eos_token = eosio::token(DICETOKEN);
        auto supply = eos_token.get_supply(eosio::symbol(DICE_SYMBOL).name());
        return supply.amount;
    }
    uint8_t random(account_name name, uint64_t game_id)
    {
        asset pool_eos = eosio::token(name("eosio.token"_n)).get_balance(receiver, eosio::symbol(S(4, EOS)).name());
        auto mixd = tapos_block_prefix() * tapos_block_num() + name + game_id - current_time() + pool_eos.amount;

        const char *mixedChar = reinterpret_cast<const char *>(&mixd);

        checksum256 result;
        sha256((char *)mixedChar, sizeof(mixedChar), &result);

        uint64_t random_num = *(uint64_t *)(&result.hash[0]) + *(uint64_t *)(&result.hash[8]) + *(uint64_t *)(&result.hash[16]) + *(uint64_t *)(&result.hash[24]);
        return (uint8_t)(random_num % 100 + 1);
    }
    void issue_token(account_name to,
                     asset quantity,
                     string memo)
    {
        st_global global = _global.get_or_default();
        auto supply = getDiceSupply();
        auto nexthalve = global.nexthalve;
        if ((DICESUPPLY - supply) <= nexthalve) // 可以减半了.
        {
            global.nexthalve = global.nexthalve * 8 / 10;
            global.eosperdice = global.eosperdice * 5 / 10;
            _global.set(global, receiver);
        }

        asset sendAmout = asset(quantity.amount * global.eosperdice, DICE_SYMBOL);

        action(permission_level{receiver, name("active"_n)},
               DICETOKEN,
               name("issue"_n),
               make_tuple(to, sendAmout, memo))
            .send();
    }

    void iplay(account_name from, asset quantity)
    {
        auto itr = _users.find(from);
        if (itr == _users.end())
        {
            _users.emplace(receiver, [&](auto &v) {
                v.amount = quantity;
                v.count = 1;
                v.owner = from;
            });
        }
        else
        {
            _users.modify(itr, 0, [&](auto &v) {
                v.amount = asset(v.amount.amount + quantity.amount, EOS_SYMBOL);
                v.count += 1;
            });
        }
    }
    void buytoken(account_name from, asset quantity)
    {
        action(permission_level{receiver, name("active"_n)},
               DICETOKEN,
               name("transfer"),
               std::make_tuple(receiver, from, asset(quantity.amount * 10000, DICE_SYMBOL), std::string("thanks! eosdice.vip")))
            .send();
    }

    void vipcheck(account_name from, asset quantity)
    {
        auto itr = _users.find(from);
        uint64_t amount = itr->amount.amount / 1e4;
        asset checkout = asset(0, EOS_SYMBOL);
        if (amount < 1000)
        {
            return;
        }
        else if (amount < 5000)
        {
            checkout = asset(quantity.amount * 1 / 1000, EOS_SYMBOL);
        }
        else if (amount < 10000)
        {
            checkout = asset(quantity.amount * 2 / 1000, EOS_SYMBOL);
        }
        else if (amount < 50000)
        {
            checkout = asset(quantity.amount * 3 / 1000, EOS_SYMBOL);
        }
        else if (amount < 100000)
        {
            checkout = asset(quantity.amount * 4 / 1000, EOS_SYMBOL);
        }
        else if (amount < 500000)
        {
            checkout = asset(quantity.amount * 5 / 1000, EOS_SYMBOL);
        }
        else if (amount < 1000000)
        {
            checkout = asset(quantity.amount * 7 / 1000, EOS_SYMBOL);
        }
        else if (amount < 5000000)
        {
            checkout = asset(quantity.amount * 9 / 1000, EOS_SYMBOL);
        }
        else if (amount < 10000000)
        {
            checkout = asset(quantity.amount * 11 / 1000, EOS_SYMBOL);
        }
        else if (amount < 50000000)
        {
            checkout = asset(quantity.amount * 13 / 1000, EOS_SYMBOL);
        }
        else
        {
            checkout = asset(quantity.amount * 15 / 1000, EOS_SYMBOL);
        }
        action(permission_level{receiver, name("active")},
               name("eosio.token"),
               name("transfer"),
               std::make_tuple(receiver, from, checkout, std::string("for vip")))
            .send();
    }
};

//old N() new ""_n
extern "C"
{
    void apply(uint64_t receiver, uint64_t code, uint64_t action)
    {
        print("recver:", name{receiver}, "name", name{code}, "action", name{action});
        print("===========", code, "========", action);
        if (code == receiver)
        {
            switch (action)
            {
                EOSIO_DISPATCH_HELPER(cardeos, (init)(debug))
            }
        }

        else if (code == "eosio.token"_n.value && action == "transfer"_n.value)
        {
            execute_action(name(receiver), name(code), &cardeos::transfer);
        }
    eosio_exit(0);
    }
};