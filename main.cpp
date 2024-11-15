#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
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

bool isValidDate(const string &dateStr)
{
    tm time = {};
    istringstream ss(dateStr);
    ss >> get_time(&time, "%d/%m/%Y");
    if (ss.fail())
    {
        cout << "Invalid date format...\n";
        return false;
    }

    auto parsedTime = chrono::system_clock::from_time_t(mktime(&time));
    auto now = chrono::system_clock::now();
    if (parsedTime > now)
        return true;
    cout << "Enter an upcoming date...\n";
    return false;
}

bool compareDate(const string &dateStr, const string &dateStr1)
{
    tm time1 = {}, time2 = {};
    istringstream ss1(dateStr), ss2(dateStr1);
    ss1 >> get_time(&time1, "%d/%m/%Y");
    ss2 >> get_time(&time2, "%d/%m/%Y");

    auto tp1 = chrono::system_clock::from_time_t(mktime(&time1));
    auto tp2 = chrono::system_clock::from_time_t(mktime(&time2));
    if (tp1 <= tp2)
        return true;
    return false;
}

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

void delete_transactions(userList *mainlist, string date)
{
    user *usercur = mainlist->head;
    account *acccur;
    transaction *transcur;
    transaction *previous;

    while (usercur != NULL)
    {
        acccur = usercur->acct;
        while (acccur != NULL)
        {
            if (acccur->txn != NULL)
            {
                transcur = acccur->txn->next;
                while (transcur != NULL)
                {
                    if (compareDate(transcur->date, date))
                    {
                        previous->next = transcur->next;
                        delete transcur;
                        transcur = previous;
                    }
                    previous = transcur;
                    transcur = transcur->next;
                }
                transcur = acccur->txn;
                if (compareDate(transcur->date, date))
                {
                    acccur->txn->next = transcur->next;
                    delete transcur;
                }
                acccur = acccur->next;
            }
        }

        usercur = usercur->next;
    }
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