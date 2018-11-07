#include "utils.hpp"


//two coins
#define EOS_SYMBOL symbol(symbol_code("EOS"),4)
#define DICE_SYMBOL symbol(symbol_code("BOCAI"),4)

//log n 
#define LOG name("eosbocailogs")
#define DICETOKEN name("eosbocai1111")
#define DEV name("eosbocaidevv")
#define PRIZEPOOL name("eosbocai1111")
#define DICESUPPLY 88000000000000

typedef uint32_t eostime;


struct [[eosio::table]] st_user
{
    account_name owner;
    asset amount;
    uint32_t count;
    uint64_t primary_key() const { return owner; }
};

struct [[eosio::table]] st_bet
{
    uint64_t id;
    account_name player;
    account_name referrer;
    asset amount;
    uint8_t roll_under;
    uint64_t created_at;
    uint64_t primary_key() const { return id; }
};


struct  st_result
{
    uint64_t bet_id;
    account_name player;
    account_name referrer;
    asset amount;
    uint8_t roll_under;
    uint8_t random_roll;
    asset payout;
};


//singletone 
struct st_fund_pool
{
    asset locked;
};


struct st_global
{
    uint64_t current_id;
    double eosperdice;
    uint64_t nexthalve;
    uint64_t initStatu;
};


typedef eosio::multi_index<name("users"), st_user> tb_uesrs;
typedef eosio::multi_index<name("bets"), st_bet> tb_bets;


typedef singleton<name("fundpool"), st_fund_pool> tb_fund_pool;
typedef singleton<name("global"), st_global> tb_global;
