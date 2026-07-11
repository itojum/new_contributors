= はじめに
やるぞい

== パターンマッチとは
Rubyにおけるパターンマッチとは、Rubyのオブジェクトに対して、オブジェクトの構造をパターンと照合し、マッチした値をローカル変数に束縛する機能です。
これによって、深いネスト構造を持ったオブジェクトから簡潔に値の探索・抽出が行えます。

@<list>{code3.1}は、Hashパターン(@<hd>{パターンマッチによる探索|Hashパターン})とArrayパターン(@<hd>{パターンマッチによる探索|Arrayパターン})を組み合わせたサンプルコードです。

//listnum[code3.1][パターンマッチを使ったサンプルコード][ruby]{
crew = {
	name: "アウグストゥス",
	skills: [
		{ label: "PicoRuby", level: 3 },
		{ label: "Ruby", level: 5 }
	]
}

case crew
in { name: name, skills: [*, { label: "PicoRuby", level: level }, *] }
	puts "#{name}さんは、PicoRuby レベル#{level}のスキルを持っています"
else
	puts "PicoRubyのスキルを持っていません"
end

# => アウグストゥスさんは、PicoRuby レベル3のスキルを持っています
//}

@<list>{code3.1}をパターンマッチを使わずに実装する場合、@<list>{code3.2}のように@<code>{#each}や@<code>{#find}などで繰り返し処理を行い、探索することになるでしょう。
@<list>{code3.1}と@<list>{code3.2}を比較していただければ、パターンマッチを使うことで簡潔に値の探索・抽出を行えることがわかると思います。

//listnum[code3.2][if句とeachメソッドを使ったサンプルコード][ruby]{
# crewの定義は省略

name = crew[:name]
picoruby_skill = nil

if crew[:skills]
  crew[:skills].each do |skill|
    if skill[:label] == "PicoRuby"
      picoruby_skill = skill
    end
  end
end

if picoruby_skill
  puts "#{name}さんは、PicoRuby レベル#{picoruby_skill[:level]}のスキルを持っています"
else
  puts "PicoRubyのスキルを持っていません"
end

# => アウグストゥスさんは、PicoRuby レベル3のスキルを持っています
//}

== パターンマッチによる探索
=== Valueパターン
=== Hashパターン
=== Arrayパターン
