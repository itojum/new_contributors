= 社内業務をAIで簡単改善！@<br>{}〜動かして、見て、最適化するAIエージェント開発〜

== はじめに

「ど〜しようかなあ」

急に部内でAI活用の機運が高まり、白羽の矢が立ってしまった。AIエージェントを運用してみたけれど、なんだかしっくりこない。そんな経験はありませんか。

生成AIやAIエージェントは、うまく使えれば実務を大きく改善できる可能性があります。一方で、「とりあえず動いた」だけでは、実務で継続的に使えるサービスにはなかなか近づきません。

LLMを使ったアプリケーションは、従来の決定的なプログラムとは少し違います。同じような入力でも、文脈やモデルの判断によって出力が変わることがあります。つまり、LLMには不確実性があります。

だからこそ、AIエージェントを実務で使える形に近づけるには、「一度作って終わり」ではなく、動かして、観測して、改善する考え方が重要になります。

=== この章の目的

この章の目的は、AWSのサービスであるAmazon Bedrock AgentCoreを使ったエージェント開発の全体像をつかみ、実務で使えるサービスに近づけるための技術的な流れを理解することです。

具体的には、次の内容を扱います。

 * AgentCoreでAIエージェントを動かす
 * 実行状況やログを確認できるようにする
 * 観測した結果をもとに改善する

=== 対象読者

この章は、次のような読者を想定しています。

 * 生成AIエージェントを実務で使ってみたい人
 * AgentCore Harnessに興味がある人
 * デモではなく、実務で継続的に使える形に近づけたい人
 * AWSの基本的なサービス名は聞いたことがあるが、AgentCoreはこれから触る人

=== 前提条件

この章では、AWS上でAgentCoreを動かすことを前提に進めます。

手元で一緒に試す場合は、次の環境を用意してください。

 * AWSアカウント
 * AWS CLI
 * AWS CLIで利用する認証情報
 * AgentCoreを利用できるAWSリージョン
 * AgentCoreや関連サービスを操作できるIAM権限

なお、AWSアカウントの作成、AWS CLIのインストール、認証情報の設定方法は本章では扱いません。

=== この章で扱うこと

この章では、AgentCoreを使ってAIエージェントを動かし、その実行状況を観測し、改善につなげるための入口を扱います。

 * AgentCore Harnessを使ったAIエージェントの作成
 * AgentCore Observabilityを使った実行状況の確認
 * AgentCore Optimizationを使った改善の考え方

== AI Agentを動かす

この章では、まずAIエージェントを小さく動かします。

AIエージェントは便利そうに見えますが、最初から複雑な構成にすると、どこで何が起きているのか分かりづらくなります。そこで今回は、できるだけシンプルな構成から始めます。

=== AI Agentとはなにか

AI Agentとは、LLMに役割や目的を与え、入力に応じて必要な処理を進める仕組みです。

通常のチャットでは、ユーザーの入力に対してLLMが応答します。AI Agentでは、それに加えて「何を達成したいのか」「どのような手順で考えるのか」「必要に応じてどのツールを使うのか」といった情報を与えます。

本章では、AI Agentを「目的を持って動くLLMアプリケーション」として扱います。厳密な定義に深入りするのではなく、AgentCoreで実際に動かし、観測し、改善する流れを重視します。

=== AgentCoreでAgentを動かす選択肢

AgentCoreでAI Agentを動かす方法として、ここではAgentCore RuntimeとAgentCore Harnessを取り上げます。

==== AgentCore Runtime

AgentCore Runtimeは、AI Agentの実行環境を柔軟に構成したい場合に向いています。

ツール連携や周辺システムとの統合を含めてAgentを作り込みたい場合は、Runtimeを使うことで、アプリケーションとしてのふるまいを細かく実装できます。

==== AgentCore Harness

AgentCore Harnessは、プロンプトを中心にAI Agentをすばやく作成したい場合に向いています。

最初からコードを書き込むのではなく、まずAgentの役割や指示を定義して動作を確認できます。今回のように、短い時間でAgentを作り、次の「見る」「改善する」流れにつなげたい場合に扱いやすい選択肢です。

今回は短い時間でAgentを動かし、Observabilityで実行状況を確認する流れを体験したいため、Harnessを使って進めます。

=== 今回作るAgent

今回は、郵便番号または住所を入力すると、出発地周辺からの日帰り旅行プランを提案するAgentを作ります。Agent名は @<code>{TravelPlanner} とします。

入力例は、次のようなものです。

 * @<code>{100-0005}
 * @<code>{神奈川県横浜市西区みなとみらい}
 * @<code>{123}

正しい郵便番号や住所が入力された場合は、市区町村または主要駅くらいの粒度に丸めて、日帰りの旅行プランを返します。一方で、@<code>{123} のように入力が不完全な場合は、旅行プランを作らず、郵便番号または住所の再入力を求めます。

==== プロンプトだけで作る

AgentCore Harnessでは、まずシステムプロンプトを中心にAgentを作成できます。

今回は、Agentに次のような指示を与えます。

 * 回答は日本語にする
 * 入力は日本の郵便番号または住所として扱う
 * 番地、建物名、部屋番号は回答に繰り返さない
 * 情報が曖昧な場合は、旅行プランを作らず聞き返す
 * 最新情報が必要な項目は、推測で断定しない
 * 出力形式をそろえる

特に重要なのは、住所の扱いです。システムプロンプトで「詳細住所を繰り返さない」と指示しても、入力自体はモデルやトレースに渡ります。そのため、実サービスで詳細住所を扱う場合は、Agentへ渡す前のアプリケーション層で市区町村レベルへ丸める設計が必要です。

今回はHandsONなので、テスト入力には郵便番号、市区町村、駅、公開施設など、非機微な情報だけを使います。

==== 観測・改善につなげやすい形にする

このAgentは、後続の「見る」「改善する」につなげやすいように、期待するふるまいをあらかじめ決めておきます。

たとえば、次の観点で確認します。

 * 郵便番号を出発地として解釈できているか
 * 住所を市区町村レベルに丸められているか
 * 不完全な入力に対して、無理に旅行プランを作っていないか
 * 最新情報を確定事項として書いていないか
 * 出力形式が崩れていないか

このように確認観点を先に決めておくと、Agentを動かしたあとに「何を見ればよいか」が明確になります。

=== AgentCore HarnessでAgentを作る

ここからは、AgentCore CLIを使ってHarnessを作成します。

AgentCore CLIの導入方法や各コマンドの詳細は、AWS公式ドキュメント @<href>{https://docs.aws.amazon.com/bedrock-agentcore/latest/devguide/harness-get-started.html} を参照してください。本章では、今回のHandsONで実行するコマンドと確認ポイントだけを扱います。

作業用の値として、リージョン、プロジェクト名、モデルIDを決めます。ここでは東京リージョン、@<code>{TravelPlanner}、@<code>{openai.gpt-oss-20b-1:0} を使います。以降のコマンド例はBashで実行する前提です。

//list[set-agentcore-vars][作業用の値を設定する]{
export AWS_REGION="ap-northeast-1"
export PROJECT_NAME="TravelPlanner"
export MODEL_ID="openai.gpt-oss-20b-1:0"
//}

@<code>{gpt-oss-20b} は、Bedrockで利用できるGPT-OSSのうち小さめのモデルです。今回のように、入力を判定して定型的な旅行プランを返す用途では、まず低コストなモデルで試します。日本語品質や判断精度が足りない場合に、より大きなモデルとの比較を考えます。

==== Agentの役割を決める

まず、AgentCoreプロジェクトを作成します。

//list[create-agentcore-project][AgentCoreプロジェクトを作成する]{
agentcore create \
  --name "$PROJECT_NAME" \
  --no-agent \
  --skip-git
cd "$PROJECT_NAME"
//}

==== 指示を書く

次に、Harnessを追加します。Agentに与える指示は、あらかじめ @<code>{system-prompt.txt} に書いておきます。

//list[add-agentcore-harness][Harnessを追加する]{
SYSTEM_PROMPT="$(< ../system-prompt.txt)"

agentcore add harness \
  --name "$PROJECT_NAME" \
  --model-provider bedrock \
  --model-id "$MODEL_ID" \
  --api-format converse_stream \
  --system-prompt "$SYSTEM_PROMPT"
//}

今回のプロンプトでは、入力の解釈、住所の丸め方、最新情報の扱い、出力形式を指定しています。

==== 実行する

デプロイ前に、設定を検証します。

//list[validate-agentcore-project][設定を検証する]{
agentcore validate
agentcore deploy --dry-run --json
agentcore deploy --diff
//}

差分に、意図しないVPC、永続ストレージ、追加モデル、追加ツールが含まれていないことを確認します。

問題なければ、デプロイします。

//list[deploy-agentcore-project][Harnessをデプロイする]{
agentcore deploy --yes --verbose
agentcore status --json
//}

ここから先はAWSリソースが作成され、AgentCoreやモデル利用の料金が発生し得ます。実行するアカウントとリージョンは、デプロイ前に必ず確認してください。

==== 実行結果を確認する

デプロイ後は、Amazon Bedrock AgentCore ConsoleのHarness Playgroundで動作確認します。

//image[handson/handson-01-harness-list][作成したHarnessをAgentCore Consoleで確認する][scale=0.9]{
//}

//image[handson/handson-02-harness-detail][Harnessの詳細画面でステータスやARNを確認する][scale=0.9]{
//}

対象のHarnessとして @<code>{TravelPlanner} を選び、次の入力を試します。テストケースごとに新しいセッションを開始すると、会話履歴の影響を分けて確認できます。

//image[handson/handson-03-playground][Harness Playgroundで入力を試す][scale=0.9]{
//}

 * @<code>{100-0005}
 * @<code>{神奈川県横浜市西区みなとみらい}
 * @<code>{123}

期待する挙動は次のとおりです。

 * 正しい郵便番号では、市区町村レベルの出発地を特定し、日帰りプランを提示する
 * 住所では、番地などを再掲せず、市区町村レベルに丸めてプランを提示する
 * 不完全な入力では、旅行プランを捏造せず、郵便番号または住所の再入力を求める
 * 営業時間、運賃、料金などは、利用前に再確認が必要だと明示する

この時点では、まだ良し悪しを細かく判断しきらなくてかまいません。まずは、Agentが期待した入口に立っているかを確認します。

=== 次に見るべきもの

Agentが動いたら、次に確認したいのは「期待どおりに動き続けるか」です。

LLMには不確実性があります。ある入力ではうまく答えられても、別の入力では意図と違う回答をすることがあります。また、ツール呼び出しや外部サービスとの連携が入ると、失敗の原因も増えていきます。

だからこそ、AI Agentを実務で使える形に近づけるには、実行結果を観測し、失敗や改善点を見つけられる状態にしておくことが重要です。

次の章では、AgentCore Observabilityを使って、Agentの実行状況を見ていきます。

== AI Agentを監視/評価して動きの中身を見る

=== なぜAI Agentには観測と評価が必要なのか

AI Agentは、動いた瞬間がゴールではありません。その大きな理由は、@<b>{Agentの出力に不確実性がある}ためです。

先ほど作ったAgentでも、同じ @<code>{100-0005} を入力したからといって、毎回まったく同じ旅行先や経路が返るとは限りません。似た住所でも、Agentの解釈によって異なる結果になることがあります。

実務で使う場合、この不確実性が失敗につながることがあります。そのため、実行結果を確認し、問題を見つけ、小さく直すサイクルが必要です。

しかし、何が起きたのか分からないままでは、どこを直せばよいのか判断できません。まずは改善の材料を集めるため、「見る」ことにフォーカスしていきましょう。

=== AgentCore Observability による監視

AgentCore Observabilityは、Agentの実行中に何が起きたのかを記録し、確認するための機能です。

最終応答だけを見ると、Agentの実行はひとつの処理に見えます。しかし実際には、ユーザー入力を受け取り、モデルを呼び出し、必要に応じてツールを使い、最終応答を作るという複数の処理がつながっています。

Observabilityでは、この流れをSession、Trace、Spanという単位で確認できます。Sessionは一連の会話、Traceは1回のリクエストと応答、Spanはモデル呼び出しやツール実行などの個々の処理です。

=== AgentCore Evaluations による評価

AgentCore Evaluationsは、Agentの実行結果が期待にどの程度合っていたかを評価するための機能です。

Observabilityが「何が起きたのかを残す録画」だとすれば、Evaluationsは「その結果がよかったかを判断する採点」に近い役割です。Evaluatorを選ぶことで、Helpfulnessなどの観点からAgentの実行結果を評価できます。

評価スコアは結論ではなく、改善前後を比べるための手がかりです。スコアだけで判断せず、実際の回答とTraceもあわせて確認します。

=== Observabilityを有効にする

ここからは、設定と実行をCLI、結果の確認をGUIで行います。

AgentのTraceをCloudWatchで確認するには、AWSアカウントでCloudWatch Transaction Searchを有効にします。この設定はHarness単位ではなく、リージョン内のAWSアカウント全体に適用されます。また、Spanの取り込みにはCloudWatchの利用料金が発生します。

Transaction Searchの設定方法とCLIの詳細は、AWS公式ドキュメント @<href>{https://docs.aws.amazon.com/AmazonCloudWatch/latest/monitoring/Enable-TransactionSearch.html} を参照してください。

まず、Transaction Searchの現在の状態を確認します。

//list[check-transaction-search][Transaction Searchの状態を確認する]{
aws xray get-trace-segment-destination \
  --region ap-northeast-1
//}

@<code>{Destination} が @<code>{CloudWatchLogs}、@<code>{Status} が @<code>{ACTIVE} であれば有効です。未設定の場合は、CloudWatch ConsoleのTransaction Search設定からSpanの取り込みを有効にします。

検索対象にするTraceの割合は、HandsONでは1%から始めます。

//list[configure-trace-indexing][Traceのインデックス率を設定する]{
aws xray update-indexing-rule \
  --region ap-northeast-1 \
  --name Default \
  --rule 'Probabilistic={DesiredSamplingPercentage=1}'
//}

設定後、Harness Playgroundで @<code>{TravelPlanner} を開き、新しいSessionで @<code>{100-0005} を実行します。CloudWatchへ反映されるまで数分かかる場合があります。

=== Traceを見る

CloudWatch Consoleを開き、GenAI ObservabilityからAmazon Bedrock AgentCoreを選びます。

確認する順番は次のとおりです。

 1. Agents Viewで @<code>{TravelPlanner} を探す
 2. Sessions Viewで先ほど実行したSessionを開く
 3. Traces ViewでTraceを選ぶ
 4. Timelineからモデル呼び出しのSpanを開く

//image[handson/handson-04-observability-session][CloudWatchでSessionとTraceを確認する][scale=0.9]{
//}

最初からすべての属性を読む必要はありません。まず、ユーザー入力と最終応答、処理時間、エラーの有無を確認します。次に、モデル呼び出しの時間、入出力Token数、ツールを追加した場合はツール呼び出しの成否を確認します。

今回の検証では、モデル呼び出しにかかった時間、最初のTokenが返るまでの時間、入出力Token数をTraceから確認できました。最終応答だけでなく途中の処理が見えることで、遅さや失敗の原因を切り分けやすくなります。

//image[handson/handson-05-trace-detail][Traceの詳細でSpanやイベントを確認する][scale=0.9]{
//}

=== Online Evaluationを設定する

次に、組み込みEvaluatorの @<code>{Builtin.Helpfulness} を使ったOnline EvaluationをCLIで追加します。少数の実行を確実に評価するため、HandsON中だけSampling率を100%にします。評価にはモデル利用料金が発生するため、確認後は停止します。

Online Evaluationの設定項目やEvaluatorの指定方法は、AWS公式ドキュメント @<href>{https://docs.aws.amazon.com/bedrock-agentcore/latest/devguide/create-online-evaluations.html} を参照してください。

//list[add-online-evaluation][Online Evaluationを追加する]{
agentcore add online-eval \
  --name TravelPlannerQuality \
  --evaluator Builtin.Helpfulness \
  --sampling-rate 100 \
  --log-group-name aws/spans \
  --service-name harness_TravelPlanner_TravelPlanner.DEFAULT \
  --enable-on-create \
  --json

agentcore validate
agentcore deploy --yes --verbose
//}

デプロイ後、Harness Playgroundで新しいSessionを開始し、もう一度 @<code>{100-0005} を実行します。EvaluationはSessionの終了を待って処理されるため、結果が表示されるまで数分かかる場合があります。

結果はCloudWatch GenAI Observabilityの対象Agentから確認します。回答内容と評価結果を見比べ、「交通経路に誤りがないか」「未確認情報を断定していないか」など、次に直す問題をひとつ選びます。

確認後は、継続的な評価を停止します。

//list[pause-online-evaluation][Online Evaluationを停止する]{
agentcore pause online-eval TravelPlannerQuality --json
//}

ここまでで、Agentを動かし、実行を観測し、評価を設定するところまで進みました。次は、見つけた問題をどのように改善するかを考えます。

== AI Agentの動きを見て最適化する

ここまでで、Agentを動かし、実行を観測して、評価を設定しました。ここからは、見つかった問題を一度にひとつだけ直し、同じ条件でもう一度確かめます。

=== AgentCore Optimization による最適化

AgentCore Optimizationは、Agentのトレースや評価結果を材料に、システムプロンプトやツール説明の改善案を作成する機能です。人がログを一件ずつ読み解く作業を補助し、改善候補を見つけやすくします。

ただし、提案は正解ではありません。特に実務で使うAgentでは、セキュリティ、権限、業務ルール、責任範囲に照らして、人が採用の可否を判断します。Optimizationは判断を置き換えるものではなく、判断材料を増やす機能と考えるとよいでしょう。

Optimizationの設定方法は、AWS公式ドキュメント @<href>{https://docs.aws.amazon.com/bedrock-agentcore/latest/devguide/optimization.html} を参照してください。


=== 観測結果から改善点を決める

まず、最終回答とTraceから問題をひとつ選びます。今回のAgentにはWeb検索ツールがないため、最新の運賃、営業時間、経路などをモデルの知識だけで回答すると、もっともらしい誤情報が含まれる可能性があります。

そこで、「最新性が必要な情報は断定せず、確認が必要であることを明記する」というルールが守られているかを確認します。守られていなければ、システムプロンプトへ条件を追加または具体化します。ツールを利用するAgentであれば、必要な場面で正しいツールを選べているかも確認し、曖昧なツール説明を見直します。

変更は一度にひとつに絞ります。複数箇所を同時に変えると、どの変更が結果に影響したのか判断しにくくなるためです。

=== 変更してもう一度確かめる

改善内容を @<code>{system-prompt.txt} へ反映し、CLIで設定を検証してデプロイします。その後、Harness Playgroundで改善前と同じ @<code>{100-0005} を入力します。

最終回答だけでなくTraceも開き、応答内容、実行時間、モデル呼び出しなどを改善前と比較します。Online Evaluationが利用できる構成では、同じEvaluatorによるスコアも比較します。狙った問題が直り、別の問題が増えていないことを確認できたら、次の改善へ進みます。

このように「動かす、見る、ひとつ直す、同じ条件で確かめる」を短く繰り返すことで、LLMの不確実性を残したままでも、Agentを実務で扱いやすい状態へ近づけられます。

== おわりに

この章では、AgentCoreを使ってAI Agentを「動かす」だけで終わらせず、実行結果を「見る」ことで状態を把握し、評価やトレースをもとに「改善する」流れを確認しました。

実務で使う際には移り変わるビジネス要件や改善要求を満たすために高速にPDCAを回さないといけない場面が多々あるかとおもいます。また、実装工数が限られているなかで最大限の成果を出す必要もあると思います。

本章では、そのような厳しい要件下でも実装できるような実装例を紹介いたしました。本章が一人でも多くのAI Agentライフを彩れればと思います。
