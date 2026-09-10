import React from 'react';
import Layout from '@theme/Layout';
import Link from '@docusaurus/Link';

export default function Home(): JSX.Element {
  return (
    <Layout title="Project Durian Chronicle" description="Project Durian 개발 위키와 개발일지">
      <main className="container margin-vert--lg">
        <h1>Project Durian Chronicle</h1>
        <p>Project Durian의 개발 위키와 개발일지입니다.</p>
        <Link className="button button--primary" to="/docs/intro">
          개발 위키 보기
        </Link>
      </main>
    </Layout>
  );
}
