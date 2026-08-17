const form = document.querySelector('#waitlist');
const note = document.querySelector('#form-note');

form?.addEventListener('submit', (event) => {
  event.preventDefault();
  const email = new FormData(form).get('email');
  if (!email) return;
  note.textContent = 'SIGNAL RECEIVED / WAITLIST ENDPOINT READY';
  note.style.color = 'var(--signal)';
  form.reset();
});

const observer = new IntersectionObserver((entries) => {
  entries.forEach((entry) => {
    if (entry.isIntersecting) entry.target.classList.add('is-visible');
  });
}, { threshold: 0.12 });

document.querySelectorAll('section, .feature-card, .arch-row, .research-grid article').forEach((el) => {
  el.style.transition = 'opacity .7s ease, transform .7s ease';
  el.style.opacity = '0';
  el.style.transform = 'translateY(16px)';
  observer.observe(el);
});

document.addEventListener('scroll', () => {
  document.querySelectorAll('.is-visible').forEach((el) => {
    el.style.opacity = '1';
    el.style.transform = 'translateY(0)';
  });
}, { passive: true });

window.dispatchEvent(new Event('scroll'));
