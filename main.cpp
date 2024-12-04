#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <windows.h>
using namespace std;

#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

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

void print_euro(const wchar_t* str)
{
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), str, wcslen(str), NULL, NULL);
}

bool isValidDate(const string &dateStr)
{
    tm time = {};
    istringstream ss(dateStr);
    ss >> get_time(&time, "%d/%m/%Y");
    if (ss.fail())
    {
        cout <<RED<< "Invalid date format...\n"<<RESET;
        return false;
    }
    return true;

    // auto parsedTime = chrono::system_clock::from_time_t(mktime(&time));
    // auto now = chrono::system_clock::now();
    // if (parsedTime > now)
    //     return true;
    // cout << "Enter an upcoming date...\n";
    // return false;
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

string getCurrentDate()
{
    auto now = std::time(nullptr);
    std::tm *localTime = std::localtime(&now);

    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%d/%m/%Y", localTime);
    return string(buffer);
}

double subtractdate(string date1, string date2)
{
    tm time1 = {}, time2 = {};
    istringstream ss1(date1), ss2(date2);
    ss1 >> get_time(&time1, "%d/%m/%Y");
    ss2 >> get_time(&time2, "%d/%m/%Y");

    std::chrono::system_clock::time_point tp1 = chrono::system_clock::from_time_t(mktime(&time1));
    std::chrono::system_clock::time_point tp2 = chrono::system_clock::from_time_t(mktime(&time2));
    chrono::duration<double> dif = tp1 - tp2;

    return dif.count() / 86400;
}

userList *input_data(userList *mainlist)
{
    ifstream txtfile("database.txt");
    string line, temp;
    bool firstuser = true;
    bool firstacc = true;
    bool firsttxn = true;
    user *currentuser;
    account *currentacc;
    transaction *currenttxn;
    int i;
    while (getline(txtfile, line))
    {
        istringstream iss(line);
        if (line[0] == '-')
        {
            firstacc = true;
            user *newuser = new user;

            if (!firstuser)
            {
                currentuser->next = newuser;
                newuser->previous = currentuser;
            }
            else
            {
                firstuser = false;
                mainlist->head = newuser;
                newuser->previous = NULL;
            }
            currentuser = newuser;

            getline(iss, temp, ',');
            currentuser->userID = stoi(line.substr(1, temp.length() - 1));
            getline(iss, currentuser->fname, ',');
            getline(iss, currentuser->lname, '\n');
            currentuser->next = NULL;
            currentuser->acct = NULL;
        }
        else if (line[0] == '#')
        {
            account *newacc = new account;
            firsttxn = true;

            if (!firstacc)
                currentacc->next = newacc;
            else
            {
                firstacc = false;
                currentuser->acct = newacc;
            }
            currentacc = newacc;
            getline(iss, temp, ',');
            currentacc->IBAN = temp.substr(1, temp.length() - 1);

            getline(iss, currentacc->accountName, ',');

            getline(iss, temp, ',');
            for (i = 0; i < temp.length(); i++)
            {
                if (!isdigit(temp[i]))
                {
                    break;
                }
            }
            currentacc->balance = stod(temp.substr(0, i));
            currentacc->currency = temp.substr(i, temp.length() - i);

            getline(iss, temp, ',');
            for (i = 0; i < temp.length(); i++)
            {
                if (!isdigit(temp[i]))
                {
                    break;
                }
            }
            currentacc->limitDepositPerDay = stod(temp.substr(0, i));

            getline(iss, temp, '\n');
            for (i = 0; i < temp.length(); i++)
            {
                if (!isdigit(temp[i]))
                {
                    break;
                }
            }
            currentacc->limitWithdrawPerMonth = stod(temp.substr(0, i));
            currentacc->next = NULL;
            currentacc->txn = NULL;
        }

        else if (line[0] == '*')
        {
            transaction *newtxn = new transaction;
            if (!firsttxn)
                currenttxn->next = newtxn;
            else
            {
                firsttxn = false;
                currentacc->txn = newtxn;
            }
            currenttxn = newtxn;

            getline(iss, temp, ',');
            currenttxn->date = temp.substr(1, temp.length() - 1);
            getline(iss, temp, '\n');
            for (i = 1; i < temp.length(); i++)
            {
                if (!isdigit(temp[i]))
                {
                    break;
                }
            }
            currenttxn->amount = stod(temp.substr(0, i));
            currenttxn->next = NULL;
        }
        else
        {
            cout << "Invalid input file..." << endl;
            exit(1);
        }
    }
    mainlist->tail = currentuser;
    return mainlist;
}

double convert(double amount, string sendercur, string targetcur)
{
    if (sendercur == "$" && targetcur == "€")
        return amount * 0.92;
    else if (sendercur == "$" && targetcur == "L.L")
        return amount * 90000;
    else if (sendercur == "L.L" && targetcur == "€")
        return convert(amount * 0.92, "$", "€");
    else if (sendercur == "L.L" && targetcur == "$")
        return amount / 90000;
    else if (sendercur == "€" && targetcur == "$")
        return amount / 0.92;
    else if (sendercur == "€" && targetcur == "L.L")
        return convert(amount / 0.92, "$", "L.L");
    return amount;
}

void transfer(userList *mainlist, double amount, account *sender, account *target)
{
    user *usercur = mainlist->head;
    account *acccur;
    transaction *txn;
    double sum = 0, sub;
    bool found = false;
    string curdate = getCurrentDate();

    while (usercur != NULL)
    {
        acccur = usercur->acct;
        while (acccur != NULL)
        {
            if (acccur->IBAN == sender->IBAN)
            {
                txn = acccur->txn;
                while (txn != NULL)
                {
                    sub = subtractdate(curdate, txn->date);
                    if (sub <= 30 && txn->amount < 0)
                        sum -= txn->amount;
                    txn = txn->next;
                }
                if (acccur->balance >= amount && acccur->limitWithdrawPerMonth >= sub)
                {
                    found = true;
                    break;
                }
                else
                {
                    cout << RED << "Insufficient balance or limit..." << RESET <<endl;
                    return;
                }
            }
            acccur = acccur->next;
        }
        if (found)
            break;
        usercur = usercur->next;
    }
    if (!found)
    {
        cout << RED << "Sender account not found..." << RESET <<endl;
        return;
    }

    found = false;
    sum = 0;
    usercur = mainlist->head;
    account *targetacc;
    double amount2;

    while (usercur != NULL)
    {
        targetacc = usercur->acct;
        while (targetacc != NULL)
        {
            if (targetacc->IBAN == target->IBAN)
            {
                while (txn != NULL)
                {
                    sub = subtractdate(curdate, txn->date);
                    if (sub <= 1 && txn->amount > 0)
                        sum += txn->amount;
                    txn = txn->next;
                }
                if (targetacc->limitDepositPerDay >= sum)
                {
                    found = true;
                    break;
                }
                else
                {
                    cout << RED << "Limit exceeded..." << RESET << endl;
                    return;
                }
            }
            targetacc = targetacc->next;
        }
        if (found)
            break;
        usercur = usercur->next;
    }
    if (!found)
    {
        cout << RED << "Target account not found..." << RESET << endl;
        return;
    }
    else
    {
        transaction *send = new transaction;
        transaction *receive = new transaction;
        amount2 = convert(amount, acccur->currency, targetacc->currency);

        acccur->balance -= amount;
        send->amount = -amount;
        send->date = curdate;
        send->next = acccur->txn;
        acccur->txn = send;

        targetacc->balance += amount2;
        receive->amount = amount2;
        receive->date = curdate;
        receive->next = targetacc->txn;
        targetacc->txn = receive;
    }

    cout<<GREEN<<"Transaction completed successfully..."<<RESET<<endl;
}

userList *sort(userList *mainlist)
{
    user *usercur = mainlist->head;
    account *acccur;
    transaction *txn, *head;
    transaction *prev, *temp, *maxdate, *tprev;
    bool swap;

    while (usercur != NULL)
    {
        acccur = usercur->acct;
        while (acccur != NULL)
        {
            txn = acccur->txn;
            head = txn;
            maxdate = txn;
            while (txn != NULL)
            {
                prev = txn;
                temp = txn->next;
                swap = false;
                while (temp != NULL)
                {
                    if (compareDate(maxdate->date, temp->date))
                    {
                        tprev = prev;
                        maxdate = temp;
                        swap = true;
                    }
                    prev = prev->next;
                    temp = temp->next;
                }
                if (swap && compareDate(txn->date, temp->date))
                {
                    tprev->next = maxdate->next;
                    maxdate->next = head;
                }
                else if (swap && txn != head)
                {
                    tprev->next = txn->next;
                    maxdate = txn;
                    maxdate->next = head;
                }
                txn = txn->next;
            }
            acccur = acccur->next;
        }
        usercur = usercur->next;
    }

    return mainlist;
}

userList *add_account(userList *mainlist, user *client, account *temp)
{
    user *usercur = mainlist->head;
    account *acccur;
    account *newacc = new account;
    newacc = temp;
    newacc->next = NULL;
    while (usercur != NULL)
    {
        if (usercur == client)
        {
            acccur = usercur->acct;
            if (acccur == NULL)
            {
                cout << "Account added successfully..." << endl;
                usercur->acct = newacc;
                return mainlist;
            }

            while (acccur->next != NULL)
            {
                if (acccur->IBAN == newacc->IBAN)
                {
                    cout << "Account already exists..." << endl;
                    return mainlist;
                }
                acccur = acccur->next;
            }

            if (acccur->IBAN == newacc->IBAN)
            {
                cout << "Account already exists..." << endl;
                return mainlist;
            }

            acccur->next = newacc;
            cout << "Account added successfully..." << endl;
            return mainlist;
        }

        usercur = usercur->next;
    }

    cout << "User not found..." << endl;
    return mainlist;
}

userList *create_account(userList *mainlist)
{
    //TODO
    return mainlist;
}

userList *create_user(userList *mainlist)
{
    //TODO
    return mainlist;
}

int display_accounts(userList *mainlist)
{
    user *usercur = mainlist->head;
    account *acccur;
    transaction *txncur;
    bool accounts = true;
    int i = 1;
    cout << fixed << setprecision(2);
    while (usercur != NULL)
    {
        acccur = usercur->acct;
        while (acccur != NULL)
        {
            accounts = false;
            cout<<"\tAccount "<<i++<<":"<<endl;
            cout<<"\tIBAN: "<<acccur->IBAN<<endl;
            cout<<"\tName: "<<acccur->accountName<<endl;
            cout<<"\tCurrency: ";
            if (acccur->currency != "$" && acccur->currency != "L.L")
            {
                print_euro(L"€\n");
            }
            else
            {
                cout<<acccur->currency<<endl;
            }
            cout<<"\tBalance: "<<acccur->balance<<endl;
            cout<<"\tLimit deposit per day: "<<acccur->limitDepositPerDay<<endl;
            cout<<"\tLimit withdraw per month: "<<acccur->limitWithdrawPerMonth<<endl<<endl;
            acccur = acccur->next;
        }
        usercur = usercur->next;
    }
    if (accounts)
    {
        cout<<RED<<"No accounts found..."<<RESET<<endl;
        return 0;
    }
    return 1;
    
}

userList *delete_transactions(userList *mainlist, string date)
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
                    acccur->txn = transcur->next;
                    delete transcur;
                }
                acccur = acccur->next;
            }
        }

        usercur = usercur->next;
    }
    return mainlist;
}

void export_data(userList *mainlist)
{
    ofstream txtfile("database.txt");
    user *usercur = mainlist->head;
    account *acccur;
    transaction *txncur;
    txtfile << fixed << setprecision(1);
    while (usercur != NULL)
    {
        txtfile << "-" << usercur->userID << "," << usercur->fname << "," << usercur->lname << "\n";
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
    userList *mainlist = new userList;
    mainlist = input_data(mainlist);
    int input;
    char inputchar;
    bool error;
    string date;
    double amount;
    cout<<"Welcome to Cedars Bank...\n";
    while (true)
    {
        error = false;
        do
        {
            if (error)
            {
                cout<<RED<<"Invalid input..."<<RESET<<endl;
            }
            cout<<"How can we help you?\n";
            cout<<setw(5)<<1<<" Create a new user...\n";
            cout<<setw(5)<<2<<" Create a new account...\n";
            cout<<setw(5)<<3<<" Transfer money to another account...\n";
            cout<<setw(5)<<4<<" Delete transactions before a specified date...\n";
            cout<<setw(5)<<0<<" Exit application...\n";
            cout<<"-> ";
            cin>>input;
            error = true;
        } while (input < 0 || input > 4);
        cin.ignore();
        if (input == 0)
        {
            break;
        }
        else if (input == 1)
        {
            mainlist = create_user(mainlist);
        }
        else if (input == 2)
        {
            mainlist = create_account(mainlist);
        }
        else if (input == 3)
        {
            input = display_accounts(mainlist);
            if (input == 1)
            {
                account acc1, acc2;
                cout<<"Enter the IBAN of the sender:\n";
                cout<<"-> ";
                getline(cin, acc1.IBAN);
                cout<<"Enter the IBAN of the receiver:\n";
                cout<<"-> ";
                getline(cin, acc2.IBAN);
                do
                {
                    cout<<"Enter the amount to send:"<<endl;
                    cout<<"-> ";
                    cin>>amount;
                } while (amount < 0);
                transfer(mainlist, amount, &acc1, &acc2);
            }
        }
        else if (input == 4)
        {
            do
            {
                cout<<"Enter a specified date (dd/mm/yyyy)\n";
                cout<<"-> ";
                getline(cin, date);
            } while (!isValidDate(date));
            
            error = false;
            do
            {
                if(error)
                {
                    cout<<RED<<"Invalid input..."<<RESET<<endl;
                }
                cout<<"This action cannot be undone, do you want to proceed? [y/n]: "<<endl;
                cout<<"-> ";
                cin>>inputchar;
                error = true;
            } while (tolower(inputchar) != 'y' && tolower(inputchar) != 'n');

            if (tolower(inputchar) == 'y')
            {
                cout<<GREEN<<"Proceeding..."<<endl;
                mainlist = delete_transactions(mainlist, date);
                cout<<"All transactions before "<<date<<" were successfully deleted..."<<RESET<<endl;
            } 
        }
    }
    cout<<GREEN<<"Thanks for using our services..."<<endl<<"Exiting app..."<<RESET<<endl;
    export_data(mainlist);
}