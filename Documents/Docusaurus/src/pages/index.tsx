import React from 'react';
import Layout from '@theme/Layout';
import Link from '@docusaurus/Link';

export default function Home(): JSX.Element {
  return (
    <Layout title="Project Durian" description="Project Durian 개발 문서">
      <main className="container margin-vert--lg">
        <h1>Project Durian</h1>
        <p>Project Durian의 개발 문서와 개발일지입니다.</p>
        <Link className="button button--primary" to="/docs/intro">
          문서 보기
        </Link>
      </main>
    </Layout>
  );
}
