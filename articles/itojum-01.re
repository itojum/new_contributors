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

=== Hashパターン


=== Findパターン

=== Arrayパターン

=== Alternativeパターン

=== Variableパターン

=== Valueパターン

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
