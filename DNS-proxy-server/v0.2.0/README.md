Second alpha version of the DNS server. Changes include:
  -use of a hashtable instead of a linked list for holding clients;
  -implementation of a hostname parser to extract the hostname in string format from a DNS query;
  -use of a configuration file for storing variables that need changing (such as server adressess and port numbers) without the need for recompiling the source code;
  -implementation of a configuration file parser;
