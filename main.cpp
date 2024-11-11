#include <iostream>
#include <fstream>
#include <string>
using namespace std;

struct transaction
{
    string date;
    double amount;
    transaction *next;
};

struct account
{
    string IBAN;
    string accountName;
    double balance;
    string currency;
    double limitDepositPerDay;
    double limitWithdrawPerMonth;
    transaction *txn;
    account *next;
};

struct user
{
    int userID;
    string fname;
    string lname;
    account *acct;
    user *next, *previous;
};

struct userList
{
    user *head, *tail;
};

void input_data()
{
}

void transfer(userList *mainlist, double amount, account *sender, account *target)
{
    // Ma 3refet sewe shi
    // hahahaha
}

int main()
{
    input_data();
}