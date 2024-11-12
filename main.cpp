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

void add_account(userList *mainlist, user *client, account *newacc)
{

    // NOTE: ma baaref iza zabta ma sawet testing, baaden mensewe l testing...

    user *usercur = mainlist->head;
    account *acccur;
    while (usercur != NULL)
    {
        if (usercur == client)
        {
            acccur = usercur->acct;
            while (acccur->next != NULL)
            {
                if (acccur->IBAN == newacc->IBAN)
                {
                    cout << "Account already exists..." << endl;
                    return;
                }
                acccur = acccur->next;
            }

            if (acccur->IBAN == newacc->IBAN)
            {
                cout << "Account already exists..." << endl;
                return;
            }

            acccur->next = newacc;
            cout << "Account added successfully..." << endl;
            return;
        }

        usercur = usercur->next;
    }

    cout << "User not found..." << endl;
    return;
}

int main()
{
    input_data();
}