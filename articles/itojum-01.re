= コードが綺麗にできるRubyのパターンマッチをおすすめしたい！
== はじめに
「このコード、もっと綺麗に書けないかな」

深くネストしたHashやArrayから値を取り出すコードを書いていると、そう思うことはありませんか。条件分岐を重ねるうちに、最初に何をしたかったのかが見えにくくなりがちです。

Rubyにはそんなときに使えるパターンマッチという機能があります。オブジェクトの構造をそのまま条件として書けるので、複雑なデータからの値の抽出がすっきり書けます。Ruby 2.7で実験的機能として導入され、Ruby 3.0で正式機能になりました。本章ではRuby 4.0.5を使用しています。

=== この章の目的

パターンマッチの仕組みを理解して、実際のコードに使えるようになることです。

具体的には次の内容を扱います。

 * パターンマッチの基本的な考え方
 * 代表的なパターンの種類と使い方
 * 自作クラスへのパターンマッチの適用
 * パターンマッチを使ったリファクタリング

=== 対象読者

次のような方を想定しています。

 * Rubyを書いたことがある人
 * パターンマッチを聞いたことはあるが使ったことがない人
 * 複雑な条件分岐やデータ抽出のコードをすっきりさせたい人

=== この章で扱うこと

 * パターンマッチの基本構文（@<code>{case/in}）
 * Value・Variable・Hash・Array・Find・Alternativeパターン
 * @<code>{#deconstruct}と@<code>{#deconstruct_keys}による自作クラスへの適用
 * パターンマッチを使ったリファクタリングの例

=== この章で扱わないこと

 * パターンマッチの全仕様の網羅
 * 一行パターンマッチ（@<code>{=>}や@<code>{in}単体の使い方）
 * Rubyのバージョンごとの詳細な差異
 * パフォーマンスの比較

== パターンマッチとは
Rubyにおけるパターンマッチとは、Rubyのオブジェクトに対して、オブジェクトの構造をパターンと照合し、マッチした値をローカル変数に束縛する機能です。
これによって、深いネスト構造を持ったオブジェクトから簡潔に値の探索・抽出が行えます。
パターンマッチはRuby 2.7で実験的機能として導入され、Ruby 3.0で正式機能になりました。従来の@<code>{case/when}が値の単純な比較に使われるのに対し、パターンマッチでは@<code>{case/in}を使い、オブジェクトの構造そのものを条件として記述できます。

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
	"#{name}さんは、PicoRuby レベル#{level}のスキルを持っています"
else
	"PicoRubyのスキルを持っていません"
end

# => "Bobさんは、PicoRuby レベル3のスキルを持っています"
//}

== パターン図鑑
パターンマッチで使えるパターンは複数あります。ここでは、それらを例を交えながらご紹介します。

=== Valueパターン
Valueパターンは値を(@<code>{#===})で比較したときに@<code>{true}になるオブジェクトにマッチします。
@<code>{#===}はRubyの「ケース等値演算子」で、クラスに対して使うとそのインスタンスかどうかを、RangeやRegexpに対して使うと包含・マッチするかどうかを判定します。
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
単体で使うことは少なく、HashパターンやArrayパターンと組み合わせてネストした構造から値を取り出す際に力を発揮します。値が不要な場合は@<code>{_}を使うと「何でもマッチするが束縛しない」という意味になります。
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
パターンに指定したキーのみチェックされ、他のキーは無視されます。APIのレスポンスのように多くのキーを持つHashから必要なキーだけを取り出す際に特に便利です。
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
@<code>{*}を使うと残りの要素をまとめてマッチさせることができます。@<code>{*rest}のように変数名を付けると残りの要素を配列として束縛することもできます。
//listnum[code3.5][Arrayパターンのサンプルコード][ruby]{
crews = ["Alice", "Bob", "Charlie", "Dave"]
case crews
in ["Alice", next_user, *]
  "Aliceさんの後ろの従業員は#{next_user}さん"
else
  "マッチしなかったよ..."
end
# => "Aliceさんの後ろの従業員はBobさん"
//}

=== Findパターン
FindパターンはArrayオブジェクトか後述の@<code>{#deconstruct}メソッド(@<hd>{クラスオブジェクトのパターンマッチ|#deconstruct})を持つオブジェクトにマッチします。
Arrayパターン(@<hd>{パターン図鑑|Arrayパターン})との違いは、Arrayパターンが先頭か末尾が固定なのに対し、Findパターンは配列のどこかにあればマッチします。配列の順序を問わず特定の要素を探したい場合に適しています。
//listnum[code3.6][Findパターンのサンプルコード][ruby]{
crews = ["Alice", "Bob", "Charlie", "Dave"]
case crews
in [*, "Charlie" ,*]
  "Charlieさんがいました！"
else
  "マッチしなかったよ..."
end
# => "Charlieさんがいました！"
//}

=== Alternativeパターン
Alternativeパターンは複数のパターンを組み合わせて「OR」でマッチします。
@<code>{|}記号で複数のパターンを並べることができます。

//listnum[code3.7][Alternativeパターンのサンプルコード][ruby]{
role = "director"

case role
in "manager" | "director" | "executive"
  "管理職です"
else
  "メンバーです"
end
# => "管理職です"
//}

== クラスオブジェクトのパターンマッチ
Hashパターンは@<code>{#deconstruct_keys}が定義されたオブジェクトに、ArrayパターンとFindパターンは@<code>{#deconstruct}が定義されたオブジェクトにもマッチします。
自作クラスにこれらのメソッドを定義することで、パターンマッチの恩恵をHashやArray以外のオブジェクトにも適用できます。

=== #deconstruct
@<code>{#deconstruct}メソッドはArrayパターンやFindパターンに使われます。オブジェクトの属性を配列として返すよう定義することで、自作クラスのインスタンスをArrayパターンでマッチできるようになります。
//listnum[code3.8][#deconstructのサンプルコード][ruby]{
class Crew
  def initialize(name, role, age)
    @name = name
    @role = role
    @age  = age
  end

  def deconstruct
    [@name, @role, @age]
  end
end

crew = Crew.new("Alice", "engineer", 22)

case crew
in Crew["Alice", role, age]
  "Aliceの役職は #{role} 、年齢は #{age} です"
else
  "マッチしなかったよ..."
end
# => "Aliceの役職は engineer 、年齢は 22 です"
//}

=== #deconstruct_keys
@<code>{#deconstruct_keys}メソッドはHashパターンに使われます。引数@<code>{keys}にはパターンで指定されたキーの配列が渡されるので、必要なキーだけを返すよう最適化することもできますが、通常はすべてのキーを含むHashを返せば問題ありません。

//listnum[code3.9][#deconstruct_keysのサンプルコード][ruby]{
class Crew
  def initialize(name, role, age)
    @name = name
    @role = role
    @age  = age
  end

  def deconstruct_keys(keys)
    { name: @name, role: @role, age: @age }
  end
end

crew = Crew.new("Bob", "manager", 35)

case crew
in Crew(role: "manager", name:)
  "マネージャーの #{name} さん"
else
  "マッチしなかったよ..."
end
# => "マネージャーの Bob さん"
//}

== パターンマッチによるリファクタリング

@<list>{code3.10}は、GitHubのAPIでアサインされていない かつ 'good first issue'ラベルが割り当てられているissueを取得するコードです。
やりたいことに対して複雑すぎると思いませんか？これをパターンマッチでリファクタリングしてみましょう。
//listnum[code3.10][初心者向けのIssuesを取得するメソッド][ruby]{
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

@<list>{code3.11}ではリファクタリングをするために先ほど紹介したいくつかのパターンを使っています。

 * Hashパターン(@<hd>{パターン図鑑|Hashパターン})
 * Findパターン(@<hd>{パターン図鑑|Findパターン})
 * Arrayパターン(@<hd>{パターン図鑑|Arrayパターン})
 * Valueパターン(@<hd>{パターン図鑑|Valueパターン})
 * Variableパターン(@<hd>{パターン図鑑|Variableパターン})

@<list>{code3.10}と@<list>{code3.11}を比較していただければ、パターンマッチを使うことで簡潔に値の探索・抽出を行えることがわかると思います。

//listnum[code3.11][パターンマッチをしてリファクタリングしたコード][ruby]{
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
      html_url: url, title: title
    }
      result_issues << { url:, title: }
    else
    end
  end

  return result_issues
end
//}

== おわりに

この章では、パターンマッチの基本的な考え方から、Value・Variable・Hash・Array・Find・Alternativeと各パターンの使い方、自作クラスへの適用、そして実際のコードへのリファクタリングまでを見てきました。

パターンマッチは覚えることが多いですが、一度使い始めると複雑なデータ構造を扱うコードがすっきり書けるようになります。まずはHashパターンあたりから試してみてください。

パターンマッチが皆さんのRubyライフを少し豊かにできれば嬉しいです。
