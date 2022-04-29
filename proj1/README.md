# Project 1

evaulation: 13/15
- -1 point because of application of patch to reformate output

Script for analyzing COVID-19 cases in Czech republic

## Usage

```bash
./corona -h
```

- displays help where usage is properly described

## File handle

### Valid .csv file format
```bash
id,datum,vek,pohlavi,kraj_nuts_kod,okres_lau_kod,nakaza_v_zahranici,nakaza_zeme_csu_kod,reportovano_khs
```

### Testing files

osoby-short.csv - shortened version of oficial osoby.csv file (https://onemocneni-aktualne.mzcr.cz/api/v2/covid-19/osoby.csv) CAREFULLY! it has 250+ MB

osoby2.csv - file with poorly formatted data, for testing purposes, script should handle all this inputs correctly

### Multiple files

Script handles multiple files on input by merging them into one big file and removing all the headers except for the one in first file

### Empty file

Script handles empty file by displaying only its header

Example:

```bash
cat /dev/null | ./corona
```

### No file

Script handles this scenario by taking the data from STDIN

Example for first 5 lines of osoby-short.csv:

```bash
cat osoby-short.csv | head -n 5 | ./corona
```

### Compressed file handle

Script can decompress .gz and .bz2 files

## Validation

Script validates flags, commands and file suffixes given by user and their values, also checks for valid date and age inside the file

Empty columns will be aggregated to None on output except for countries command

## TODO

- add zadani