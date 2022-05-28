#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include <regex>

using namespace std;

// baseurl + work + book + chapter + verse
const regex regexp(
        "(?:\\d\\s*)?[A-Z\\]?[a-z&]+\\s*\\d+(?:[:-]\\d+)?(?:\\s*-\\s*\\d+)?(?::\\d+|(?:\\s*[A-Z]?[a-z]+\\s*\\d+:\\d+))?");

const static string INPUT_FILE = "input.txt";
const static string OUTPUT_FILE = "viewHTML.html";

//FIXME remove this useless but debugging function
void PrintInputStream(istream &readStream) {
  string temp;
  getline(readStream, temp);
  while (!readStream.fail()) {
    cout << temp << endl;
    getline(readStream, temp);
  }
}

//FIXME move the definition of this function to the bottom and leave a prototype here
string FindReferenceInString(string lineToRead) {
  string temp = lineToRead;
  smatch m;
  regex_search(lineToRead, m, regexp);
  for (auto x: m)
    cout << x << " ";
  return "";
}

const vector<vector<string> > OTHER_NAMES = {
        {"genesis", "exodus",  "leviticus",            "numbers",              "deuteronomy", "joshua", "judges",          "ruth",          "1 samuel",  "2 samuel",  "1 kings",     "2 kings",    "1 chronicles",    "2 chronicles",    "ezra",      "nehemiah",  "esther", "job",      "psalms",  "proverbs", "ecclesiastes", "song of solomon", "isaiah", "jeremiah", "lamentations", "ezekiel", "daniel", "hosea", "joel", "amos", "obadiah", "jonah", "micah", "nahum", "habakkuk", "zephaniah", "haggai", "zechariah", "malachi"},
        {"matthew", "mark",    "luke",                 "john",                 "acts",        "romans", "1 corinthians",   "2 corinthians", "galatians", "ephesians", "philippians", "colossians", "1 thessalonians", "2 thessalonians", "1 timothy", "2 timothy", "titus",  "philemon", "hebrews", "james",    "1 peter",      "2 peter",         "1 john", "2 john",   "3 john",       "jude",    "revelations"},
        {"1 nephi", "2 nephi", "jacob",                "enos",                 "jarom",       "omni",   "words of mormon", "mosiah",        "alma",      "helaman",   "3 nephi",     "4 nephi",    "mormon",          "ether",           "moroni"},
        {"section", "official declaration"},
        {"moses",   "abraham", "joseph smith matthew", "joseph smith history", "articles of faith"}
};
const vector<vector<string> > SITE_BOOK_NAMES = {
        {"gen",   "ex",   "lev",  "num",  "deut", "josh", "judg",  "ruth",  "1-sam", "2-sam", "1-kgs",  "2-kgs", "1-chr",  "2-chr",  "ezra",  "neh",   "esth",  "job",    "ps",  "prov",  "eccl",  "song",  "isa",  "jer",  "lam",  "ezek", "dan", "hosea", "joel", "amos", "obad", "jonah", "micah", "nahum", "hab", "zeph", "hag", "zech", "mal"},
        {"matt",  "mark", "luke", "john", "acts", "rom",  "1-cor", "2-cor", "gal",   "eph",   "philip", "col",   "1-thes", "2-thes", "1-tim", "2-tim", "titus", "philem", "heb", "james", "1-pet", "2-pet", "1-jn", "2-jn", "3-jn", "jude", "rev"},
        {"1-ne",  "2-ne", "jacob",  "enos",  "jarom",  "omni",  "w-of-m",    "mosiah",   "alma",   "hel",   "3-ne",   "4-ne",  "morm",   "ether",    "moro"},
        {"dc",    "od"},
        {"moses", "abr",  "js-m", "js-h", "a-of-f"}
};
const string SITE_WORK_NAMES[5] = {
        "ot", "nt", "bofm", "dc-testament", "pgp"
};
const string scripBaseURL = "https://www.churchofjesuschrist.org/study/scriptures/";

string generateURL(int work, int book, int chapter, int verse, int verse2 = -1) {
  return scripBaseURL + SITE_WORK_NAMES[work] + "/" + SITE_BOOK_NAMES[work][book] + "/" + to_string(chapter) +
         "?lang=eng&id=" + to_string(verse) + (verse2 != -1 ? ("-" + to_string(verse2)) : "") + "#p" + (verse > 1 ? to_string(verse-1) : to_string(verse));
}

string generateReference(string book, int chapter, int verse, int verse2 = -1) {
  int bookInt = -1, work = -1;
  transform(book.begin(), book.end(), book.begin(),
            [](unsigned char c) { return tolower(c); });
  string temp;
  for (int i = 0; i < OTHER_NAMES.size(); i++) {
    for (int j = 0; j < OTHER_NAMES[i].size(); j++) {
      temp = OTHER_NAMES[i][j];
      if (book == temp) {
        bookInt = j;
        work = i;
      }
    }
  }
  if (bookInt == -1 && work == -1) {
    for (int i = 0; i < SITE_BOOK_NAMES.size(); i++) {
      for (int j = 0; j < SITE_BOOK_NAMES[i].size(); j++) {
        temp = SITE_BOOK_NAMES[i][j];
        if (book == temp) {
          bookInt = j;
          work = i;
        }
      }
    }
  }
  if (bookInt == -1 && work == -1) {
    cout << "reference not found" << endl;
    return "";
  }
  string generated = generateURL(work, bookInt, chapter, verse, verse2);
  // cout << generated << endl;
  return generated;
}

string decodeReference(string r) {
  string verse2, chapter, book;
  string verse = r.substr(r.find(':') + 1);
  string leftover = r.substr(0,r.find(':'));
  for (int i = 0; i < verse.length(); i++) {
    if (verse.substr(i,1) == "-") {
      verse2 = verse.substr(i+1,verse.length()-i-1);
      verse = verse.substr(0,i);
    }
  }
  for (int i = leftover.length() - 1; i >= 0; i--) {
    if (leftover.substr(i,1) == " ") {
      chapter = leftover.substr(i + 1,leftover.length() - i);
      book = leftover.substr(0,i);
      break;
    }
  }
  if (chapter.length()==0 || book.length()==0) {
    cout << "no reference" << endl;
    return "";
  }
  try {
    string temp;
    if (verse2.length() != 0) {
      temp = generateReference(book,stoi(chapter),stoi(verse),stoi(verse2));
    } else {
      temp = generateReference(book,stoi(chapter),stoi(verse));
    }
    cout << "Link to " << r << " -> " << temp << endl;

  } catch (exception&){
    cout << "no reference found for " << r << endl;
  }
  return verse;
}

//FIXME move the definition of this function to the bottom and leave a prototype here
vector<string> FindReferencesInInputStream(istream &readStream) {
  string currentLine;
  vector<string> refs;
  getline(readStream, currentLine);
  string temp;
  while (!readStream.fail()) {
    temp += decodeReference(currentLine);
    getline(readStream, currentLine);
  }
}

int main() {
//  cout << generateURL(0,0,1,1,4) << endl;
//  return 0;
  cout << "list your references on a text file in this directory called \"" << INPUT_FILE << "\"." << endl;
  ifstream readFile;
  ofstream createdHTML;

  readFile.open(INPUT_FILE);
  if (!readFile.is_open()) {
    cout << INPUT_FILE << " file doesnt exist or isn't working. Try again by creating that text file again and then re-run."
         << endl;
    return 1;
  } //error checking to read the input file
  cout << "Successfully read from input file" << endl;

  createdHTML.open(OUTPUT_FILE);
  if (!createdHTML.is_open()) {
    cout << "failed to create " << OUTPUT_FILE << ". Good luck." << endl;
    return 1;
  } //error checking to read the html file
  cout << "Successfully created or found output file before writing" << endl;

  //run the loop to read the file and find scripture references
  vector<string> results = FindReferencesInInputStream(readFile);

  // todo write results to the output file
  return 0;
}
