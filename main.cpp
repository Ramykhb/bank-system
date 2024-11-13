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

void export_data(userList *mainlist)
{
    ofstream txtfile("input.txt");
    user *usercur = mainlist->head;
    account *acccur;
    transaction *txncur;
    while (usercur != NULL)
    {
        txtfile << "-" << usercur->fname << "," << usercur->lname << "\n";
        acccur = usercur->acct;
        while (acccur != NULL)
        {
            txtfile << "#" << acccur->IBAN << "," << acccur->accountName << "," << acccur->balance << acccur->currency << "," << acccur->limitDepositPerDay << acccur->currency << "," << acccur->limitWithdrawPerMonth << acccur->currency << "\n";
            txncur = acccur->txn;
            while (txncur != NULL)
            {
                txtfile << "*" << txncur->date << "," << txncur->amount << acccur->currency << "\n";
                txncur = txncur->next;
            }
            acccur = acccur->next;
        }
        usercur = usercur->next;
    }
    txtfile.close();
}

int main()
{
    input_data();
}