#!/usr/bin/env perl

my $str = join('', <>);
my $comment_re = qr[(?:\A|(?<!\\))(?:/\*.*?\*/|//[^\r\n]*)]ms;
my $non_comment_re = qr[(?:\A|(?<!\\))((["'])(?:\\.|(?:(?!\2|\\).))*\2)]ms;

$str =~ s[$non_comment_re|$comment_re][$1]gms;
$str =~ s[$non_comment_re|[[:blank:]]+(?:(?=\r)|$)][$1]gms;
$str =~ s[$non_comment_re|^[\r\n]+][$1]gms;

print $str;
