= はじめに
やるぞい

== パターンマッチとは
Rubyにおけるパターンマッチとは、Rubyのオブジェクトに対して、オブジェクトの構造をパターンと照合し、マッチした値をローカル変数に束縛する機能です。
これによって、深いネスト構造を持ったオブジェクトから簡潔に値の探索・抽出が行えます。

//listnum[code3.1][パターンマッチを使ったサンプルコード][ruby]{
crew = {
	name: "Bob",
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

# => Bobさんは、PicoRuby レベル3のスキルを持っています
//}

== パターン図鑑
パターンマッチで使えるパターンは複数あります。ここでは、それらを例を交えながらご紹介します。

=== Valueパターン
Valueパターンは値を(@<code>{#===})で比較したときに@<code>{true}になるオブジェクトにマッチします。
@<list>{code3.2}のサンプルコードは、@<code>{Integer === 168 # => true}になるためマッチとなります。

//listnum[code3.2][Valueパターンのサンプルコード][ruby]{
case 168
in Integer
	"マッチしたよ"
else
	"マッチしなかったよ..."
end
# => "マッチしたよ"
//}


=== Variableパターン
Variableパターンは、対象の値がなんであってもマッチし、その値を新しい変数に束縛します。
先頭が小文字かアンダースコア(_)で始まる名前を書くと新しい変数として束縛されます。
//listnum[code3.3][Variableパターンのサンプルコード][ruby]{
value = "Alice"
case value
in name
	"#{name}さん！こんにちは！"
else
	"マッチしなかったよ..."
end
# => "Aliceさん！こんにちは！"
//}

=== Hashパターン
HashパターンはHashオブジェクトか後述の@<code>{#deconstruct_keys}メソッド(@<hd>{クラスオブジェクトのパターンマッチ|#deconstruct_keys})を持つオブジェクトにマッチします。
@<list>{code3.4}はValueパターン(@<hd>{パターン図鑑|Valueパターン})と組み合わせています。

//listnum[code3.4][Hashパターンのサンプルコード][ruby]{
case { value: 168 }
in { value: Integer }
	"マッチしたよ"
else
	"マッチしなかったよ..."
end
# => "マッチしたよ"
//}
=== Arrayパターン
ArrayパターンはArrayオブジェクトか後述の@<code>{#deconstruct}メソッド(@<hd>{クラスオブジェクトのパターンマッチ|#deconstruct})を持つオブジェクトにマッチします。
//listnum[code3.5][Hashパターンのサンプルコード][ruby]{
crews = ["Alice", "Bob", "Charlie", "Dave"]
case crews
in ["Alice", next_user, *]
  "Aliceさんの後ろの従業員は#{next_user}さん"
else
	puts "マッチしなかったよ..."
end
# => "Aliceさんの後ろの従業員はBobさん"
//}

=== Findパターン
FindパターンはArrayオブジェクトか後述の@<code>{#deconstruct}メソッド(@<hd>{クラスオブジェクトのパターンマッチ|#deconstruct})を持つオブジェクトにマッチします。
Arrayパターン(@<hd>{パターン図鑑|Arrayパターン})との違いは、Arrayパターンが先頭か末尾が固定なのに対し、Findパターンは配列のどこかにあればマッチします。使う分には気にしなくて良いと思います。
//listnum[code3.6][Findパターンのサンプルコード][ruby]{
crews = ["Alice", "Bob", "Charlie", "Dave"]
case crews
in [*, "Charlie" ,*]
  "Charlieさんがいました！"
else
	puts "マッチしなかったよ..."
end
# => "Charlieさんがいました"
//}

=== Alternativeパターン
Alternativeパターンは複数のパターンを組み合わせて「OR」でマッチします。
@<code>{|}記号で複数のパターンを並べることができます。

//listnum[code3.7][Alternativeパターンのサンプルコード][ruby]{
role = "director"

case role
in "manager" | "director" | "executive"
  puts "管理職です"
else
  puts "メンバーです"
end
# => "管理職です"
//}

== クラスオブジェクトのパターンマッチ
=== #deconstruct
=== #deconstruct_keys

== パターンマッチによるリファクタリング

@<list>{code3.2}は、GitHubのAPIでアサインされていない かつ 'good first issue'ラベルが割り当てられているissueを取得するコードです。
やりたいことに対して複雑すぎると思いませんか？これをパターンマッチでリファクタリングしてみましょう。
//listnum[code3.2][初心者向けのIssuesを取得するメソッド][ruby]{
def fetch_unassigned_beginner_issues(repo)
  client = Octokit::Client.new(
    :access_token => ENV['GITHUB_TOKEN'],
    auto_paginate: true
  )
  issues = client.search_issues("repo:#{repo} is:open is:issue")
  result_issues = []

  issues.items.each do |issue|
    next unless (issue[:assignees] || []).empty?

    labels = issue[:labels]
    has_target_label = labels.any? do |label|
      label[:name] == 'good first issue'
    end
    next unless has_target_label

    url = issue[:html_url]
    title = issue[:title]

    result_issues << { url:, title: }
  end

  return result_issues
end
//}

@<list>{code3.3}ではリファクタリングをするために先ほど紹介したいくつかのパターンを使っています。
- Hashパターン(@<hd>{パターン図鑑|Hashパターン})
- Findパターン(@<hd>{パターン図鑑|Findパターン})
- Valueパターン(@<hd>{パターン図鑑|Valueパターン})
- Variableパターン(@<hd>{パターン図鑑|Variableパターン})

@<list>{code3.2}と@<list>{code3.3}を比較していただければ、パターンマッチを使うことで簡潔に値の探索・抽出を行えることがわかると思います。

//listnum[code3.3][パターンマッチをしてリファクタリングしたコード][ruby]{
def fetch_unassigned_beginner_issues(repo)
  client = Octokit::Client.new(
    :access_token => ENV['GITHUB_TOKEN'],
    auto_paginate: true
  )
  issues = client.search_issues("repo:#{repo} is:open is:issue")

  result_issues = []

  issues.items.each do |issue|
    case issue.to_h
    in {
      assignees: [],
      labels: [*, { name: 'good first issue' }, *],
      html_url: url, title:　title
    }
      result_issues << { url:, title: }
    else
    end
  end

  return result_issues
end
//}
